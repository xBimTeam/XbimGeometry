using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests.TestFiles
{
    [TestClass ]
    // [DeploymentItem("TestFiles")]
    public class IfcExtrudedAreaSolidTests
    {
        static private IXbimGeometryEngine geomEngine;
        static private ILoggerFactory loggerFactory;
        static private ILogger logger;

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<IfcAdvancedBrepTests>();
        }
        [ClassCleanup]
        static public void Cleanup()
        {
            loggerFactory = null;
            geomEngine = null;
            logger = null;
        }
        [TestMethod]
        public void arbritary_closed_profile_with_intersecting_voids_test()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(arbritary_closed_profile_with_intersecting_voids_test)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcBooleanClippingResult found");
                var solidSet = geomEngine.CreateSolidSet(er.Entity, logger);
                Assert.IsTrue(solidSet.Count == 1, "This solid set should have 1 solid");
                Assert.IsTrue(solidSet.First().Faces.Count == 28, "This solid should have 28 faces");
            }

        }

        [TestMethod]
        public void IfcExtrudedAreaSolidInvalidPlacementTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(IfcExtrudedAreaSolidInvalidPlacementTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcExtrudedAreaSolid found");
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                Assert.IsTrue(solid.Faces.Count == 6, "This solid should have 6 faces");
            }

        }

        [DataTestMethod]
        [DataRow("SweptDiskSolid_1", 15902.721708130202, DisplayName = "Directrix is polyline")]
        [DataRow("SweptDiskSolid_2", 5720688.107912736, DisplayName = "Directrix is trimmed")]
        [DataRow("SweptDiskSolid_3", 129879.77474359272, DisplayName = "Directrix is indexed polyline")]
        [DataRow("SweptDiskSolid_4", 129879.77474359272, DisplayName = "Ifc reference test")]
        public void SweptDiskSolidTest(string fileName, double requiredVolume)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\{fileName}.ifc"))
            {
                var sweptDisk = model.Instances.OfType<IIfcSweptDiskSolid>().FirstOrDefault();
                sweptDisk.Should().NotBeNull();
                var solid = geomEngine.CreateSolid(sweptDisk, logger);
                solid.Should().NotBeNull();
                solid.Volume.Should().BeApproximately(requiredVolume, 1e-7);
            }
        }

        [DataTestMethod]
        [DataRow("SweptDiskSolidPolygonal_1", 84336.694898618269, DisplayName = "IFC SweptDiskSolidPolygonal reference test")]
        public void SweptDiskSolidPolygonalTest(string fileName, double requiredVolume)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\{fileName}.ifc"))
            {
                var sweptSolid = model.Instances.OfType<IIfcSweptDiskSolid>().FirstOrDefault();
                sweptSolid.Should().NotBeNull();
                var sweptDiskSolid = geomEngine.CreateSolid(sweptSolid);
                sweptDiskSolid.Should().NotBeNull();
                sweptDiskSolid.Volume.Should().BeApproximately(requiredVolume, 1e-7);
            }
        }

    }
}
