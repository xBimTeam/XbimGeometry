using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.Diagnostics;
using Xbim.Common.Geometry;
using Xbim.Common.Model;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;
using Xunit;
namespace Xbim.Geometry.Engine.Tests
{

    public class GithubIssuesTests

    {
        private readonly ILoggerFactory _loggerFactory;

        public GithubIssuesTests(ILoggerFactory loggerFactory)
        {
            _loggerFactory = loggerFactory;
        }
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Github_Issue_281(XGeometryEngineVersion engineVersion)
        {
            // this file resulted in a stack-overflow exception due to precision issues in the data.
            // We have added better exception management so that the stack-overflow is not thrown any more, 
            // however the voids in the wall are still not computed correctly.
            //
            using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
            {
                m.LoadStep21("TestFiles\\Github\\Github_issue_281_minimal.ifc");
                
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                var result = c.CreateContext(null, false);

                result.Should().Be(true);
            }
        }

        [Fact]
        public void Github_Issue_447()
        {
            // This file contains a trimmed curve based on ellipse which has semiaxis1 < semiaxis2
            // and trimmed curve is parameterized with cartesian points.
            // This test checks for a bug in XBimCurve geometry creation procedure when incorrect parameter values
            // are calculated for these specific conditions described above.
            using (var model = MemoryModel.OpenRead(@"TestFiles\Github\Github_issue_447.ifc"))
            {
                var shape = model.Instances.OfType<IIfcTrimmedCurve>().FirstOrDefault();
                shape.Should().NotBeNull();
                var trimPoint1 = shape.Trim1.OfType<IIfcCartesianPoint>().FirstOrDefault();
                trimPoint1.Should().NotBeNull();
                var trimPoint2 = shape.Trim2.OfType<IIfcCartesianPoint>().FirstOrDefault();
                trimPoint2.Should().NotBeNull();

                var trimStart = new XbimPoint3D(trimPoint1.X, trimPoint1.Y, trimPoint1.Z);
                var trimEnd = new XbimPoint3D(trimPoint2.X, trimPoint2.Y, trimPoint2.Z);

                IXbimGeometryEngine geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateCurve(shape);
                geom.Should().NotBeNull();

                trimStart.Should().BeEquivalentTo(geom.Start);
                trimEnd.Should().BeEquivalentTo(geom.End);
            }
        }

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Github_Issue473(XGeometryEngineVersion engineVersion)
        {
            // Performance of v6 engine very slow for complex BREPs, but much faster using the xbim Tesselator and skipping OCC.
            using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
            {
                m.LoadStep21(@"TestFiles\Github\Github_issue_473_minimal.ifc");
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                var timer = new Stopwatch();
                timer.Start();
                var result = c.CreateContext(null, false, false);
                timer.Stop();

                result.Should().Be(true);
                c.ShapeInstances().Should().HaveCount(4);
                c.ShapeGeometries().Should().HaveCount(4);

                timer.ElapsedMilliseconds.Should().BeLessThan(10000);
            }
        }

        [Fact(Skip = "Triggers OCC Memory violation")]
        public void Github_Issue_512()
        {
            var ifcFile = @"TestFiles\Github\Github_issue_512.ifc";
            // Triggers OCC Memory violation
            using (var m = MemoryModel.OpenRead(ifcFile))
            {
                var c = new Xbim3DModelContext(m);
                var result = c.CreateContext(null, true);

                result.Should().BeTrue("Expect success");

                m.GeometryStore.IsEmpty.Should().BeFalse("Store expected to be full");
            }
        }


        [Fact]
        public void Github_Issue_512b()
        {
            var ifcFile = @"TestFiles\Github\Github_issue_512b.ifc";

            using var m = MemoryModel.OpenRead(ifcFile);
            var c = new Xbim3DModelContext(m);
            var result = c.CreateContext(null, true);
            
            result.Should().BeTrue("Expect success");

            m.GeometryStore.IsEmpty.Should().BeFalse("Store expected to be full");

            using (var reader = m.GeometryStore.BeginRead())
            {
                var regions = reader.ContextRegions.Where(cr => cr.MostPopulated() != null).Select(cr => cr.MostPopulated());

                var region = regions.FirstOrDefault();

                region.Size.Length.Should().BeApproximately(expectedValue: 0.77227, 0.001);
            }
        }


        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Cutting_Issue(XGeometryEngineVersion engineVersion)
        {

            using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
            {
                m.LoadStep21("TestFiles\\Github\\Dormitory-ARC_Opening_444.ifc");
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                c.CreateContext(null, false);

                var store = m.GeometryStore as InMemoryGeometryStore;

                var geom = store.ShapeGeometries.Values.First(c => c.IfcShapeLabel == 13519);

                geom.FaceCount.Should().Be(58);
                geom.Length.Should().Be(2221);

            }
        }


    }
}
