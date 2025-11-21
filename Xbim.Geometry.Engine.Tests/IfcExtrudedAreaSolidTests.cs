using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using System.Linq;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Geometry.Exceptions;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;
namespace Xbim.Geometry.Engine.Tests
{

    // [DeploymentItem("TestFiles")]
    public class IfcExtrudedAreaSolidTests
    {

        private ILogger _logger;
        private readonly IXGeometryConverterFactory _geomConverterFactory;
        private ILoggerFactory _loggerFactory;
        public IfcExtrudedAreaSolidTests(ILoggerFactory loggerFactory, ILogger<IfcExtrudedAreaSolidTests> logger, IXGeometryConverterFactory geomConverterFactory)
        {
            _loggerFactory = loggerFactory;
            _logger = logger;
            _geomConverterFactory = geomConverterFactory;
        }

        /// <summary>
        /// This is a precast concrete plank with holes through it, but all 6 holes are on top of each other
        /// </summary>
        [Fact]
        public void arbritary_closed_profile_with_intersecting_voids_test()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(arbritary_closed_profile_with_intersecting_voids_test)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var v6GeomEngine = _geomConverterFactory.CreateGeometryEngineV6(er.Entity.Model, _loggerFactory);
                var v6Shape = v6GeomEngine.Build(er.Entity);
                var solidSet = geomEngine.CreateSolidSet(er.Entity, _logger);
                solidSet.Count.Should().Be(1, "This solid set should have 1 solid");
                solidSet.First().Faces.Count.Should().Be(28);
                v6Shape.AllFaces().Count().Should().Be(28);
            }

        }

        /// <summary>
        /// In the test an axis placement has a null location, this is ilegal, version V5 and V6 throw different exceptions
        /// </summary>
        [Fact]
        public void IfcExtrudedAreaSolidInvalidPlacementTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(IfcExtrudedAreaSolidInvalidPlacementTest)))
            {
                var v6GeomEngine = _geomConverterFactory.CreateGeometryEngineV6(er.Entity.Model, _loggerFactory);
                var error = Assert.Throws<XbimGeometryServiceException>(() => v6GeomEngine.Build(er.Entity));
                error.Message.Should().StartWith("Error building geometry shape");

                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                er.Entity.Should().NotBeNull();

                var v5Error = Assert.Throws<XbimGeometryFactoryException>(() => geomEngine.CreateSolid(er.Entity, _logger));
                v5Error.Message.Should().Be("Error badly defined axis");
            }

        }

        
        [Theory]
        [InlineData("SweptDiskSolid_1", 4951.174655723391)]
        [InlineData("SweptDiskSolid_2", 5720687.83036694)]
        [InlineData("SweptDiskSolid_4", 129879.77474359272)]
        [InlineData("rebar_isolated", 400843.7131002933)]
        [InlineData("GL_ClippedReba-stripped", 473432.87638158724)]
        [InlineData("bar.stripped", 499561.93942171149)]
        [InlineData("bar2.stripped", 54395.84019374512)]
        public void SweptDiskSolidTest(string fileName, double requiredVolume)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\{fileName}.ifc"))
            {
                var sweptDisk = model.Instances.OfType<IIfcSweptDiskSolid>().FirstOrDefault();
                sweptDisk.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var solid = geomEngine.CreateSolid(sweptDisk, _logger);
                var str = solid.ToBRep;
                solid.Should().NotBeNull();
                solid.Volume.Should().BeApproximately(requiredVolume, 1e-7);
            }
        }


        [Theory]
        [InlineData("SweptDiskSolidPolygonal_1", 93146.73219678485)]
        public void SweptDiskSolidPolygonalTest(string fileName, double requiredVolume)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\{fileName}.ifc"))
            {
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var sweptSolid = model.Instances.OfType<IIfcSweptDiskSolidPolygonal>().FirstOrDefault();
                sweptSolid.Should().NotBeNull();
                var sweptDiskSolid = geomEngine.CreateSolid(sweptSolid, _logger);
                sweptDiskSolid.Should().NotBeNull();
                sweptDiskSolid.Volume.Should().BeApproximately(requiredVolume, 1e-7);
            }
        }

        [Theory]
        [InlineData("CurveParametersDegrees", 4228625577.2508564)]
        public void ExtrudedAreaSolidTest(string fileName, double requiredVolume)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\{fileName}.ifc"))
            {
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var sweptSolid = model.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(e => e.EntityLabel == 135);
                sweptSolid.Should().NotBeNull();
                var sweptDiskSolid = geomEngine.CreateSolid(sweptSolid, _logger);
                sweptDiskSolid.Should().NotBeNull();
                sweptDiskSolid.Volume.Should().BeApproximately(requiredVolume, 1e-7);
            }
        }


        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void IfcCShapeProfileDefGirthTest(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\test_rebro.ifc"))
            {
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory, new Interop.Configuration.GeometryEngineOptions { GeometryEngineVersion = engineVersion});
                var extrudedAreaSolid = model.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault();
                extrudedAreaSolid.Should().NotBeNull();
                var solid = geomEngine.CreateSolid(extrudedAreaSolid, _logger);
                solid.Should().NotBeNull();
            }
        }

        [Fact]
        public void can_build_empty_rectangle_profile_extrusion()
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\empty_rectangle_profile_extrusion.ifc"))
            {
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var sweptSolid = model.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault();
                sweptSolid.Should().NotBeNull();
                
                var error = Assert.Throws<XbimGeometryServiceException>(() => geomEngine.Create(sweptSolid, _logger));
                error.Message.Should().StartWith("Error building geometry shape");
                error.InnerException.Message.Should().Be("Invalid rectangle profile with at least one zero or less dimension");
            }
        }
    }
}
