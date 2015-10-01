using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.IO.Esent;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.ProfileResource;
using Xbim.Ifc2x3.IO;

namespace GeometryTests
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
                using (var m = new EsentModel(new Xbim.Ifc2x3.EntityFactory()))
                {
                    m.CreateFrom("SolidTestFiles\\5- IfcRevolvedAreaSolid-IfcCircularHollowProfileDef.ifc", null, null, true, true);
                    var ss = m.Instances.OfType<IfcRevolvedAreaSolid>().FirstOrDefault(e => e.SweptArea is IfcCircleHollowProfileDef);
                    Assert.IsTrue(ss != null, "No Revolved Area found");
                    Assert.IsTrue(ss.SweptArea is IfcCircleHollowProfileDef, "Incorrect profiledef found");

                    
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
                using (var m = new XbimModel())
                {
                    m.CreateFrom(@"SolidTestFiles\BIM Logo-Coordination View 2 - No M.ifc", null, null, true, true);
                    var ss = m.Instances.OfType<IfcRevolvedAreaSolid>().FirstOrDefault(e => e.EntityLabel==290);
                    Assert.IsTrue(ss != null, "No Revolved Area found");
                  //  m.ModelFactors.DeflectionAngle = 0.1;
                  //  m.ModelFactors.DeflectionTolerance = 0.1;
                    var solid = _xbimGeometryCreator.CreateSolid(ss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                    var facetedSphere = _xbimGeometryCreator.CreateFacetedBrep(m, solid);
                    var shell = _xbimGeometryCreator.CreateShell(facetedSphere.Outer);
                    var xbimFacetedSphere = _xbimGeometryCreator.CreateSolidSet(facetedSphere);
                    Assert.IsTrue(xbimFacetedSphere.Count==1);
                    //_xbimGeometryCreator.WriteTriangulation(Console.Out,solid, 0.001,0.5);
                  
                }
            }
        }

    }
}
