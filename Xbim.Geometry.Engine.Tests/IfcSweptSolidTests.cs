using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]
    public class IfcSweptSolidTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void IfcSweptDisk_With_IfcPolyline()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
        
                using (var m = IfcStore.Open("SolidTestFiles\\6- IfcSweptDiskSolid_With_BooleanResult.ifc"))
                {
                    var ss = m.Instances.OfType<IIfcSweptDiskSolid>().FirstOrDefault(e => e.Directrix is IIfcPolyline);
                    Assert.IsTrue(ss != null, "No Swept Disk found");
                    Assert.IsTrue(ss.Directrix is IIfcPolyline, "Incorrect sweep found");

                    
                    var solid = _xbimGeometryCreator.CreateSolid(ss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                   
                    Assert.IsTrue(solid.Faces.Count() == 4, "Swept disk solids along a single polyline with inner radius should have 4 faces");
                }
            }
        }

        [TestMethod]
        public void IfcSweptDisk_With_IFCCOMPOSITECURVE()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {

                using (var m = IfcStore.Open("SolidTestFiles\\SweptExtrusion.ifc"))
                {
                    var ss = m.Instances.OfType<IIfcSweptDiskSolid>().FirstOrDefault(e => e.Directrix is IIfcCompositeCurve);
                    Assert.IsTrue(ss != null, "No Swept Disk found");
                    Assert.IsTrue(ss.Directrix is IIfcCompositeCurve, "Incorrect sweep found");

                    // the test passes if changing Xbim.Geometry.Engine\XbimSolid.cpp line 1213 in function XbimSolid::Init(IIfcSweptDiskSolid^ swdSolid)
                    // to
                    //      gp_Ax2 axCircle(gp_Pnt(s.X, s.Y, s.Z), gp_Dir(0., 0., -1.));
                    // instead of the current
                    //      gp_Ax2 axCircle(gp_Pnt(s.X, s.Y, s.Z), gp_Dir(0., 0., 1.));
                    // is there a way to detect the initial tangent of the Directrix and use it there for the gp_Dir?
                    
                    var solid = _xbimGeometryCreator.CreateSolid(ss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);

                    Assert.IsTrue(solid.Faces.Count() == 9, "Swept disk should have 9 faces");
                }
            }
        }

        [TestMethod]
        public void IfcSweptDisk_With_IfcComposite()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\6- IfcSweptDiskSolid_With_BooleanResult.ifc"))
                {
                    var ss = m.Instances.OfType<IIfcSweptDiskSolid>().FirstOrDefault(e => e.Directrix is IIfcCompositeCurve);
                    Assert.IsTrue(ss != null, "No Swept Disk found");
                    Assert.IsTrue(ss.Directrix is IIfcCompositeCurve, "Incorrect sweep found");

                    
                    var solid = _xbimGeometryCreator.CreateSolid(ss);
                   
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 4, "Swept disk solids along this composite curve should have 4 faces");
                }
            }
        }
        [TestMethod]
        public void BIM_Logo_LetterM_Test()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\BIM Logo-LetterM.ifc"))
                {
                    var eas = m.Instances[57] as IIfcSurfaceCurveSweptAreaSolid;
                    Assert.IsTrue(eas != null, "No IfcSurfaceCurveSweptArea Solid found");

                    var solid = (IXbimSolid)xbimGeometryCreator.Create(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 26, "Letter M should have 26 faces");
                    //var xbimTessellator = new XbimTessellator(m, XbimGeometryType.PolyhedronBinary);
                    // Assert.IsTrue(xbimTessellator.CanMesh(solid));//if we can mesh the shape directly just do it
                    // var shapeGeom = xbimTessellator.Mesh(solid);
                    var geom = xbimGeometryCreator.CreateShapeGeometry(solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, m.ModelFactors.DeflectionAngle, XbimGeometryType.PolyhedronBinary);
                    Assert.IsTrue(geom.BoundingBox.Volume > 0);
                }
            }
        }

        [TestMethod]
        public void IfcSurfaceCurveSweptAreaSolid()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\11- IfcSurfaceCurveSweptAreaSolid.ifc"))
                {
                    var ss = m.Instances.OfType<IIfcSurfaceCurveSweptAreaSolid>().FirstOrDefault();
                    Assert.IsTrue(ss != null, "No Swept Disk found");
                   
                    
                    var solid = _xbimGeometryCreator.CreateSolid(ss);

                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 6, "This IfcSurfaceCurveSweptAreaSolid with hollow circular profile def should have 6 faces");
                }
            }
        }
    }
}
