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
        public void SupportMultipleProjectsAndContexts(XGeometryEngineVersion engineVersion)
        {
            
            using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
            {
                // Multiple quirks in this model:
                // 1. Has two projects and sites (& two RepresentationContexts - one of which has no sub context)
                // 2. ShapeRepresentation with an Identifier of 'Surface' for 'SurfaceModel' - when a Body is typically expected.
                m.LoadStep21(@"TestFiles\MultiProjectWithSurfaceModels.ifc");
                
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                c.BodyRepresentations.Add("surface");
                
                var result = c.CreateContext(null, false);
                result.Should().Be(true);
                c.ShapeInstances().Should().HaveCount(3);
                c.ShapeGeometries().Should().HaveCount(3);

                var wexBimFilename = @"TestFiles\MultiProjectWithSurfaceModels.wexbim";
                // Optional: Export to 'wexbim' format for use in WebUI's xViewer - geometry only
                using (var wexBimFile = File.Create(wexBimFilename))
                {
                    using (var wexBimBinaryWriter = new BinaryWriter(wexBimFile))
                    {
                        m.SaveAsWexBim(wexBimBinaryWriter);
                        wexBimBinaryWriter.Close();
                    }
                    wexBimFile.Close();
                }


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

                geom.FaceCount.Should().Be(50);
                geom.Length.Should().Be(2029);

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

        [Theory(Skip = "Broken in V6")]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Github_Issue_512_broken(XGeometryEngineVersion engineVersion)
        {
            //var loggerFactory = new LoggerFactory();
            //XbimServices.Current.ConfigureServices(s => s.AddXbimToolkit(b => b.AddLoggerFactory(loggerFactory)).AddLogging(l => l.AddConsole()));
            var ifcFile = @"TestFiles\Github\Github_issue_512.ifc";
            // Triggers OCC Memory violation
            using (var m = MemoryModel.OpenRead(ifcFile))
            {
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                var result = c.CreateContext(null, true);

                result.Should().BeTrue();

                m.GeometryStore.IsEmpty.Should().BeFalse();
            }
        }

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        //[InlineData(XGeometryEngineVersion.V6)]
        public void Github_Issue_512(XGeometryEngineVersion engineVersion)
        {
            
            //var loggerFactory = new LoggerFactory();
            //XbimServices.Current.ConfigureServices(s => s.AddXbimToolkit(b => b.AddLoggerFactory(loggerFactory)).AddLogging(l => l.AddConsole()));
            var ifcFile = @"TestFiles\Github\Github_issue_512.ifc";
            // Triggers OCC Memory violation
            using (var m = MemoryModel.OpenRead(ifcFile))
            {
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                var result = c.CreateContext(null, true);

                result.Should().BeTrue();

                m.GeometryStore.IsEmpty.Should().BeFalse();
            }
        }

        [Fact]
        public void Github_Issue_512b()
        {
            var ifcFile = @"TestFiles\Github\Github_issue_512b.ifc";
            // Triggers OCC Memory violation
            using (var m = MemoryModel.OpenRead(ifcFile))
            {
                var c = new Xbim3DModelContext(m, _loggerFactory, XGeometryEngineVersion.V5);
                var result = c.CreateContext(null, true);

                result.Should().BeTrue();

                m.GeometryStore.IsEmpty.Should().BeFalse();

                using (var reader = m.GeometryStore.BeginRead())
                {
                    var regions = reader.ContextRegions.Where(cr => cr.MostPopulated() != null).Select(cr => cr.MostPopulated());

                    var region = regions.FirstOrDefault();

                    region.Size.Length.Should().BeApproximately(0.77227, 0.001);
                }
            }
        }

        [Fact]
        public void Github_Issue_554_inconsistentLengthTreatment()
        {
            var ifcFile = @"TestFiles\Github\Github_issue_554_inconsistentLengthTreatment.ifc";
            using var m = MemoryModel.OpenRead(ifcFile);
            var c = new Xbim3DModelContext(m, _loggerFactory, XGeometryEngineVersion.V6);
            var result = c.CreateContext(null, true);
            result.Should().BeTrue();
            m.GeometryStore.IsEmpty.Should().BeFalse();
            using var reader = m.GeometryStore.BeginRead();
            var dist5meters = distanceOfPointsAlongCircle(250, 5); // linear distance of points along circle
            TestLocationDistance(reader, 200445, 200449, dist5meters);

            var dist02meters = distanceOfPointsAlongCircle(23.4, 0.2);
            TestLocationDistance(reader, 200001, 200005, dist02meters); // this fails
        }

        private double distanceOfPointsAlongCircle(double circleRadius, double distanceAlongCircumference)
        {
            var radians = distanceAlongCircumference / circleRadius;
            var p1 = new XbimPoint3D(circleRadius, 0, 0);
            var p2 = new XbimPoint3D(circleRadius * Math.Cos(radians), circleRadius * Math.Sin(radians), 0);
            return (p1 - p2).Length;
        }

        private static void TestLocationDistance(IGeometryStoreReader reader, int shapeId1, int shapeId2, double expectedDist)
        {
            var shape1 = reader.ShapeInstances.FirstOrDefault(si => si.IfcProductLabel == shapeId1);
            var shape2 = reader.ShapeInstances.FirstOrDefault(si => si.IfcProductLabel == shapeId2);
            var t1 = new XbimPoint3D(shape1.Transformation.OffsetX, shape1.Transformation.OffsetY, shape1.Transformation.OffsetZ);
            var t2 = new XbimPoint3D(shape2.Transformation.OffsetX, shape2.Transformation.OffsetY, shape2.Transformation.OffsetZ);
            var dist = (t1 - t2).Length;
            dist.Should().BeApproximately(expectedDist, 0.01);
        }

        [Fact]
        public void Github_Issue_557()
        {
            var ifcFile = @"TestFiles\Github\Github_issue_557.ifc";
            // Triggers OCC Memory violation
            using (var m = MemoryModel.OpenRead(ifcFile))
            {
                var c = new Xbim3DModelContext(m, _loggerFactory, XGeometryEngineVersion.V6);
                var result = c.CreateContext(null, true);

                result.Should().BeTrue();

                m.GeometryStore.IsEmpty.Should().BeFalse();

                using (var reader = m.GeometryStore.BeginRead())
                {
                    var regions = reader.ContextRegions.Where(cr => cr.MostPopulated() != null).Select(cr => cr.MostPopulated());

                    var region = regions.FirstOrDefault();

                    region.Size.Length.Should().BeApproximately(13.406979, 0.001);
                }
            }
        }


    }
}
