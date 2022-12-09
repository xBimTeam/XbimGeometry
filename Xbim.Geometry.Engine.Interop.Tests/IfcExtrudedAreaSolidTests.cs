using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using System.Linq;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;
namespace Xbim.Geometry.Engine.Interop.Tests.TestFiles
{
   
    // [DeploymentItem("TestFiles")]
    public class IfcExtrudedAreaSolidTests
    {

        static private ILogger logger = NullLogger<IfcExtrudedAreaSolidTests>.Instance;

        static private ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        

        /// <summary>
        /// This is a precast concrete plank with holes through it, but all the holes are on top of each other
        /// </summary>
        [Fact]
        public void arbritary_closed_profile_with_intersecting_voids_test()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(arbritary_closed_profile_with_intersecting_voids_test)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, loggerFactory);
                var solidSet = geomEngine.CreateSolidSet(er.Entity, logger);
                solidSet.Count.Should().Be(1, "This solid set should have 1 solid");
                solidSet.First().Faces.Count.Should().Be(28);
            }

        }

        [Fact]
        public void IfcExtrudedAreaSolidInvalidPlacementTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(IfcExtrudedAreaSolidInvalidPlacementTest)))
            {
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, loggerFactory);
                er.Entity.Should().NotBeNull();
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                solid.Faces.Count.Should().Be(6, "This solid should have 6 faces");
            }

        }

        [Theory]
        [InlineData("SweptDiskSolid_1", 15902.721708130202/*, DisplayName = "Directrix is polyline"*/)]
        [InlineData("SweptDiskSolid_2", 5720688.107912736/*, DisplayName = "Directrix is trimmed"*/)]
        [InlineData("SweptDiskSolid_3", 129879.77474359272/*, DisplayName = "Directrix is indexed polyline"*/)]
        [InlineData("SweptDiskSolid_4", 129879.77474359272/*, DisplayName = "Ifc reference test"*/)]
        public void SweptDiskSolidTest(string fileName, double requiredVolume)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\{fileName}.ifc"))
            {
                var sweptDisk = model.Instances.OfType<IIfcSweptDiskSolid>().FirstOrDefault();
                sweptDisk.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, loggerFactory);
                var solid = geomEngine.CreateSolid(sweptDisk, logger);
                solid.Should().NotBeNull();
                solid.Volume.Should().BeApproximately(requiredVolume, 1e-7);
            }
        }

        [Theory]
        [InlineData("SweptDiskSolidPolygonal_1", 84336.694898618269/*, DisplayName = "IFC SweptDiskSolidPolygonal reference test"*/)]
        public void SweptDiskSolidPolygonalTest(string fileName, double requiredVolume)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\{fileName}.ifc"))
            {
                var geomEngine = new XbimGeometryEngine(model, loggerFactory);
                var sweptSolid = model.Instances.OfType<IIfcSweptDiskSolid>().FirstOrDefault();
                sweptSolid.Should().NotBeNull();
                var sweptDiskSolid = geomEngine.CreateSolid(sweptSolid, logger);
                sweptDiskSolid.Should().NotBeNull();
                sweptDiskSolid.Volume.Should().BeApproximately(requiredVolume, 1e-7);
            }
        }

    }
}
