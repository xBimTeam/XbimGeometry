using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.IO;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.ProfileResource;
using Xbim.Geometry;

namespace GeometryTests
{
    [TestClass]
    [DeploymentItem(@"SolidTestFiles\")]

    public class IfcRevolvedAreaSolidTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void IfcRevolvedArea_With_IfcCircleHollowProfileDef()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("5- IfcRevolvedAreaSolid-IfcCircularHollowProfileDef.ifc", null, null, true, true);
                    var ss = m.Instances.OfType<IfcRevolvedAreaSolid>().FirstOrDefault(e => e.SweptArea is IfcCircleHollowProfileDef);
                    Assert.IsTrue(ss != null, "No Extruded Solid found");
                    Assert.IsTrue(ss.SweptArea is IfcCircleHollowProfileDef, "Incorrect profiledef found");

                    
                    var solid = _xbimGeometryCreator.CreateSolid(ss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 4, "Circle Hollow profiles should have 4 faces");
                }
            }
        }
    }
}
