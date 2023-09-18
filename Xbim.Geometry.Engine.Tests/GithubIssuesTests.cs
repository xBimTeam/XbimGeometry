using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.Diagnostics;
using Xbim.Common.Geometry;
using Xbim.Common.Model;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;
using Xunit;
namespace Xbim.Geometry.Engine.Tests
{

    public class GithubIssuesTests

    {
        private readonly ILoggerFactory _loggerFactory;
        private readonly IXbimGeometryServicesFactory _geometryfactory;

        public GithubIssuesTests(ILoggerFactory loggerFactory, IXbimGeometryServicesFactory geometryfactory)
        {
            _loggerFactory = loggerFactory;
            _geometryfactory = geometryfactory;
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

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Github_Issue_447(XGeometryEngineVersion engineVersion)
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

                var trimStart = new XbimPoint3D(trimPoint2.X, trimPoint2.Y + 360, trimPoint2.Z);
                var trimEnd = new XbimPoint3D(trimPoint1.X, trimPoint1.Y, trimPoint1.Z);

                IXbimGeometryEngine geomEngine = _geometryfactory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var geom = geomEngine.CreateCurve(shape);
                geom.Should().NotBeNull();

                trimEnd.Should().Be(geom.End);
                trimStart.Should().Be(geom.Start);
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

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Issue_483(XGeometryEngineVersion engineVersion)
        {

            using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
            {
                m.LoadStep21("TestFiles\\Github\\GitHub_issue_483_minimal.ifc");
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                c.CreateContext(null, false);

                var store = m.GeometryStore as InMemoryGeometryStore;

                var geom = store.ShapeGeometries.Values.First(c => c.IfcShapeLabel == 60035);

                geom.FaceCount.Should().Be(56);
                geom.Length.Should().Be(4317);

            }
        }


    }
}
