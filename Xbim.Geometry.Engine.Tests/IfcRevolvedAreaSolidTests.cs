using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.ProfileResource;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]
    public class IfcRevolvedAreaSolidTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void IfcRevolvedArea_With_IfcCircleHollowProfileDef()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\5- IfcRevolvedAreaSolid-IfcCircularHollowProfileDef.ifc"))
                {
                    var ss = m.Instances.OfType<IIfcRevolvedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcCircleHollowProfileDef);
                    Assert.IsTrue(ss != null, "No Revolved Area found");
                    Assert.IsTrue(ss.SweptArea is IIfcCircleHollowProfileDef, "Incorrect profiledef found");

                    
                    var solid = _xbimGeometryCreator.CreateSolid(ss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 4, "Circle Hollow profiles should have 4 faces");
                }
            }
        }

        [TestMethod]
        public void IfcRevolvedArea_To_Sphere()
        {

             using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open(@"SolidTestFiles\BIM Logo-Coordination View 2 - No M.ifc"))
                {
                    var ss = m.Instances.OfType<IIfcRevolvedAreaSolid>().FirstOrDefault(e => e.EntityLabel==290);
                    Assert.IsTrue(ss != null, "No Revolved Area found");
                  //  m.ModelFactors.DeflectionAngle = 0.1;
                  //  m.ModelFactors.DeflectionTolerance = 0.1;
                    _xbimGeometryCreator.CreateSolid(ss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                                    
                }
            }
        }

    }
}
