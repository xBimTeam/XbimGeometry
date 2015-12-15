using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.Step21;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.IO;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]
    public class SurfaceSweepTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void IfcCenterLineProfileDefTest()
        {

            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    var semiCircle = IfcModelBuilder.MakeSemiCircle(m, 20);
                    var cl = IfcModelBuilder.MakeCenterLineProfileDef(m, semiCircle, 5);
                    var face = _xbimGeometryCreator.CreateFace(cl);
                    Assert.IsNotNull(face as IXbimFace, "Wrong type returned");
                    Assert.IsTrue(((IXbimFace)face).IsValid, "Invalid face returned");
                }
            }
        }

        [TestMethod]
        public void IfcSurfaceOfLinearExtrusionTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    var semiCircle = IfcModelBuilder.MakeSemiCircle(m, 20);
                    var def = IfcModelBuilder.MakeArbitraryOpenProfileDef(m, semiCircle);
                    var cl = IfcModelBuilder.MakeSurfaceOfLinearExtrusion(m, def, 50, new XbimVector3D(0,0,1));
                    var face = _xbimGeometryCreator.CreateFace(cl);
                    Assert.IsNotNull(face as IXbimFace, "Wrong type returned");
                    Assert.IsTrue(((IXbimFace)face).IsValid, "Invalid face returned");
                }
            }
        }

        [TestMethod]
        public void IfcSurfaceOfRevolutionTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    var cc = IfcModelBuilder.MakeRationalBSplineCurveWithKnots(m);
                    var def = IfcModelBuilder.MakeArbitraryOpenProfileDef(m, cc);
                    var rev = IfcModelBuilder.MakeSurfaceOfRevolution(m, def);
                    var face = _xbimGeometryCreator.CreateFace(rev);
                    Assert.IsNotNull(face as IXbimFace,"Wrong type returned");
                    Assert.IsTrue(((IXbimFace)face).IsValid,"Invalid face returned");
                }
            }
        }
    }
}
