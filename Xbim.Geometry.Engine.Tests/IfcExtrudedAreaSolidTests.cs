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
                var error = Assert.Throws<XbimGeometryServiceException>(()=> v6GeomEngine.Build(er.Entity));
                error.Message.Should().Be("Error building geometry shape");

                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                er.Entity.Should().NotBeNull();
               
                var v5Error = Assert.Throws<XbimGeometryFactoryException>(() => geomEngine.CreateSolid(er.Entity, _logger));
                v5Error.Message.Should().Be("Error badly defined axis");
            }

        }

        /// <summary>
        /// Upgraded to V6 methods, trime accuracy improved, volumes updated
        /// </summary>
        /// <param name="fileName"></param>
        /// <param name="requiredVolume"></param>
        [Theory]
        [InlineData("SweptDiskSolid_1", 7725.7280894170744/*, DisplayName = "Directrix is polyline"*/)] //trim was not working correctly, it is now and the volume has been reduced
        [InlineData("SweptDiskSolid_2", 5552149.0343576306/*, DisplayName = "Directrix is trimmed"*/)]
       /// [InlineData("SweptDiskSolid_3", 129879.77474359272/*, DisplayName = "Directrix is indexed polyline"*/)] srl duplicate of the reference test below but with a sign error, pointless test to run
        [InlineData("SweptDiskSolid_4", 129879.77474359272/*, DisplayName = "Ifc reference test"*/)]
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
        [InlineData("SweptDiskSolidPolygonal_1", 83575.33307798137/*, DisplayName = "IFC SweptDiskSolidPolygonal reference test"*/)]
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

    }
}
