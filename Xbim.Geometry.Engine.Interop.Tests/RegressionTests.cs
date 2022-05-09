using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class RegressionTests
    {
        
        static private XbimGeometryEngine geomEngine;
        static private ILoggerFactory loggerFactory;
        static private ILogger logger;

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<Ifc4GeometryTests>();
        }
        [ClassCleanup]
        static public void Cleanup()
        {
            loggerFactory = null;
            geomEngine = null;
            logger = null;
        }

        [TestMethod]
        [Ignore]
        // // [DeploymentItem(@"TestFiles\Regression\FailingGeom.ifc", "FailingGeom.ifc")]
        public void IfcHalfspace_FailingGeom()
        {
            using (var m = IfcStore.Open("TestFiles\\Regression\\FailingGeom.ifc"))
            {
                var extSolid = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(hs => hs.EntityLabel == 185025);
                var solid = geomEngine.CreateSolid(extSolid, null);
                IfcCsgTests.GeneralTest(solid);

                var mlist = m.Instances.OfType<IIfcBooleanClippingResult>();
                foreach (var eas in mlist)
                {
                    Debug.WriteLine("Todo: " + eas.EntityLabel);
                }

                foreach (var eas in mlist)
                {
                    // var eas = m.Instances.OfType<IIfcBooleanClippingResult>().FirstOrDefault(hs => hs.EntityLabel == 185249);
                    Assert.IsTrue(eas != null, "No IfcBooleanClippingResult found");
                    var ret = geomEngine.CreateSolidSet(eas, logger);
                    
                    IfcCsgTests.GeneralTest(solid);

                    Debug.WriteLine(eas.EntityLabel + " ok");
                    if (eas.EntityLabel == 185243)
                    {
                        File.WriteAllText(@"C:\Data\_tmp\185243.v5.brep", solid.ToBRep);
                    }
                }
            }
        }
    }
}
