using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Common.Logging;
using Xbim.IO;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Geometry;

namespace GeometryTests
{
    [TestClass]
   
    public class IfcSweptSolidTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void IfcSweptDisk_With_IfcPolyline()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\6- IfcSweptDiskSolid_With_BooleanResult.ifc", null, null, true, true);
                    var ss = m.Instances.OfType<IfcSweptDiskSolid>().FirstOrDefault(e => e.Directrix is IfcPolyline);
                    Assert.IsTrue(ss != null, "No Swept Disk found");
                    Assert.IsTrue(ss.Directrix is IfcPolyline, "Incorrect sweep found");

                    
                    var solid = _xbimGeometryCreator.CreateSolid(ss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                   
                    Assert.IsTrue(solid.Faces.Count() == 4, "Swept disk solids along a single polyline with inner radius should have 4 faces");
                }
            }
        }

        [TestMethod]
        public void IfcSweptDisk_With_IfcComposite()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\6- IfcSweptDiskSolid_With_BooleanResult.ifc", null, null, true, true);
                    var ss = m.Instances.OfType<IfcSweptDiskSolid>().FirstOrDefault(e => e.Directrix is IfcCompositeCurve);
                    Assert.IsTrue(ss != null, "No Swept Disk found");
                    Assert.IsTrue(ss.Directrix is IfcCompositeCurve, "Incorrect sweep found");

                    
                    var solid = _xbimGeometryCreator.CreateSolid(ss);
                   
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 4, "Swept disk solids along a this composite curve should have 4 faces");
                }
            }
        }

        [TestMethod]
        public void IfcSurfaceCurveSweptAreaSolid()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\11- IfcSurfaceCurveSweptAreaSolid.ifc", null, null, true, true);
                    var ss = m.Instances.OfType<IfcSurfaceCurveSweptAreaSolid>().FirstOrDefault();
                    Assert.IsTrue(ss != null, "No Swept Disk found");
                   
                    
                    var solid = _xbimGeometryCreator.CreateSolid(ss);

                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 8, "This IfcSurfaceCurveSweptAreaSolid with hollow circular profile def should have 8 faces");
                }
            }
        }
    }
}
