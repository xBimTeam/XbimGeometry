using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Interop.Tests.TestFiles
{
    [TestClass ]
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
        public void IfcExtrudedAreaSolidInvalidPlacementTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(IfcExtrudedAreaSolidInvalidPlacementTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcExtrudedAreaSolid found");
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                Assert.IsTrue(solid.Faces.Count == 6, "This solid should have 6 faces");
            }

        }

        [TestMethod]
        public void IfcSweptDiskSolidWithPolylineTest()
        {
            using (var er = new EntityRepository<IIfcSweptDiskSolid>(nameof(IfcSweptDiskSolidWithPolylineTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcSweptDiskSolid found");
                
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                Assert.IsTrue(solid.Faces.Count == 5, "This solid should have 5 faces");
            }

        }
    }
}
