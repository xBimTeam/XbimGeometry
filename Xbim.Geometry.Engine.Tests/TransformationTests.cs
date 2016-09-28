using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.Step21;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]
    public class TransformationTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        /// <summary>
        /// Generally these tests transform an object, invert the transform and transform it back, checking that the start and end results are the same
        /// </summary>
        [TestMethod]
        public void TransformSolidRectangularProfileDef()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 10, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = _xbimGeometryCreator.CreateSolid(extrude);
                    var transform = new XbimMatrix3D(); //test first with identity
                    var solid2 = (IXbimSolid)solid.Transform(transform);
                    var s1Verts = solid.Vertices.ToList();
                    var s2Verts = solid2.Vertices.ToList();
                    for (int i = 0; i < s1Verts.Count; i++)
                    {
                        XbimVector3D v = s1Verts[i].VertexGeometry - s2Verts[i].VertexGeometry;
                        Assert.IsTrue(v.Length < m.ModelFactors.Precision, "vertices not the same");
                    }
                    transform.RotateAroundXAxis(Math.PI/2);
                    transform.RotateAroundYAxis(Math.PI/4);
                    transform.RotateAroundZAxis(Math.PI);
                    transform.OffsetX += 100;
                    transform.OffsetY += 200;
                    transform.OffsetZ += 300;
                    solid2 = (IXbimSolid)solid.Transform(transform);
                    Assert.IsTrue(Math.Abs(solid.Volume - solid2.Volume) < 0.001, "Volume differs");
                    transform.Invert();
                    solid2 = (IXbimSolid)solid2.Transform(transform);
                    s1Verts = solid.Vertices.ToList();
                    s2Verts = solid2.Vertices.ToList();
                    for (int i = 0; i < s1Verts.Count; i++)
                    {
                        XbimVector3D v = s1Verts[i].VertexGeometry-s2Verts[i].VertexGeometry;
                        Assert.IsTrue(v.Length < m.ModelFactors.Precision, "vertices not the same");
                    }
                    txn.Commit();
                }
            }
        }

#if USE_CARVE_CSG
        [TestMethod]
        public void TransformFacetedSolidRectangularProfileDef()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 10, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = XbimGeometryCreator.CreateSolid(extrude);
                    solid = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, 0.5);
                    var transform = new XbimMatrix3D(); //test first with identity
                    var solid2 = solid.Transform(transform);
                    var s1Verts = solid.Vertices.ToList();
                    var s2Verts = solid2.Vertices.ToList();
                    for (int i = 0; i < s1Verts.Count; i++)
                    {
                        XbimVector3D v = s1Verts[i].VertexGeometry - s2Verts[i].VertexGeometry;
                        Assert.IsTrue(v.Length < m.ModelFactors.Precision, "vertices not the same");
                    }
                    transform.RotateAroundXAxis(Math.PI / 2);
                    transform.RotateAroundYAxis(Math.PI / 4);
                    transform.RotateAroundZAxis(Math.PI);
                    transform.OffsetX += 100;
                    transform.OffsetY += 200;
                    transform.OffsetZ += 300;
                    solid2 = solid.Transform(transform);
                    Assert.IsTrue(Math.Abs(solid.Volume - solid2.Volume) < 0.001, "Volume differs");
                    transform.Invert();
                    solid2 = solid2.Transform(transform);
                    s1Verts = solid.Vertices.ToList();
                    s2Verts = solid2.Vertices.ToList();
                    for (int i = 0; i < s1Verts.Count; i++)
                    {
                        XbimVector3D v = s1Verts[i].VertexGeometry - s2Verts[i].VertexGeometry;
                        Assert.IsTrue(v.Length < m.ModelFactors.Precision, "vertices not the same");
                    }
                }
            }
        } 
#endif
    }
}
