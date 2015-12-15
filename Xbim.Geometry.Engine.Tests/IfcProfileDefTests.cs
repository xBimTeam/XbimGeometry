using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Step21;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.ProfileResource;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]
    public class IfcProfileDefTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
       
        [TestMethod]
        public void CircleProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    IfcProfileDef prof = IfcModelBuilder.MakeCircleProfileDef(m, 20);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    Assert.IsTrue(face.Area > 0);
                    txn.Commit();
                }
            }
        }

        [TestMethod]
        public void CircleHollowProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                 
                    var prof = IfcModelBuilder.MakeCircleHollowProfileDef(m, 20, 5);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    Assert.IsTrue(face.Area > 0);
                    txn.Commit();
                }
            }
        }
        [TestMethod]
        public void RectangleProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    var prof = IfcModelBuilder.MakeRectangleProfileDef(m, 20, 30);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    Assert.IsTrue(face.Area > 0);
                    txn.Commit();
                }
            }
        }

        [TestMethod]
        public void RectangleHollowProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {                   
                    var prof = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 30, 5);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    Assert.IsTrue(face.Area > 0);
                    txn.Commit();
                }
            }
        }

        [TestMethod]
        // ReSharper disable once InconsistentNaming
        public void IShapeProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    var prof = IfcModelBuilder.MakeIShapeProfileDef(m, 20, 30, 3, 4, 1);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    double area = face.Area;
                    Assert.IsTrue(face.Area > 0);
                    m.ModelFactors.ProfileDefLevelOfDetail = 1;
                    face = _xbimGeometryCreator.CreateFace(prof);

                    Assert.IsTrue(face.Area > 0);
                    Assert.IsTrue(face.OuterBound.Edges.Count == 16, "Incorrect edge count");
                    Assert.IsTrue(face.Area > area, "Detailed profile hsould be bigger than normal");
                    txn.Commit();
                }
            }
        }

        [TestMethod]
        public void LShapeProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    var prof = IfcModelBuilder.MakeLShapeProfileDef(m, 150, 90, 10, 6.0, 4.8, 0.05);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    double area = face.Area;
                    Assert.IsTrue(face.Area > 0);
                    m.ModelFactors.ProfileDefLevelOfDetail = 1;
                    face = _xbimGeometryCreator.CreateFace(prof);
                    
                    Assert.IsTrue(face.Area > 0);
                    Assert.IsTrue(face.OuterBound.Edges.Count == 9, "Incorrect edge count");
                    Assert.IsTrue(face.Area > area, "Detailed profile hsould be bigger than normal");
                    txn.Commit();
                }
            }
        }

        [TestMethod]
        public void UShapeProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    var prof = IfcModelBuilder.MakeUShapeProfileDef(m, 150, 90, 10, 6, 4.0,3, 0.05);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    double area = face.Area;
                    Assert.IsTrue(face.Area > 0);
                    m.ModelFactors.ProfileDefLevelOfDetail = 1;
                    face = _xbimGeometryCreator.CreateFace(prof);
                   
                    Assert.IsTrue(face.Area > 0);
                    Assert.IsTrue(face.OuterBound.Edges.Count == 12, "Incorrect edge count");
                    Assert.IsTrue(face.Area > area, "Detailed profile should be bigger than normal");
                    txn.Commit();
                }
            }
        }

        [TestMethod]
        public void CShapeProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (m.BeginTransaction())
                {
                    var prof = IfcModelBuilder.MakeCShapeProfileDef(m, 150, 90, 3, 10 , 3);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    double area = face.Area;
                    Assert.IsTrue(face.Area > 0);
                    m.ModelFactors.ProfileDefLevelOfDetail = 1;
                    face = _xbimGeometryCreator.CreateFace(prof);
                   
                    Assert.IsTrue(face.Area > 0);
                    Assert.IsTrue(face.OuterBound.Edges.Count == 20, "Incorrect edge count");
                    Assert.IsTrue(face.Area < area, "Detailed profile should be less than normal");
                }
            }
        }

        [TestMethod]
        // ReSharper disable once InconsistentNaming
        public void TShapeProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    var prof = IfcModelBuilder.MakeTShapeProfileDef(m, 150, 90, 13, 20, 6, 3, 4, 0.05, 0.1);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    double area = face.Area;
                    Assert.IsTrue(face.Area > 0);
                    m.ModelFactors.ProfileDefLevelOfDetail = 1;
                    face = _xbimGeometryCreator.CreateFace(prof);
                   
                    Assert.IsTrue(face.Area > 0);
                    Assert.IsTrue(face.OuterBound.Edges.Count == 14, "Incorrect edge count");
                    Assert.IsTrue(face.Area < area, "Detailed profile should be less than normal profile");
                    txn.Commit();
                }
            }
        }

        [TestMethod]
        public void ZShapeProfileDefTest()
        {
            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = m.BeginTransaction())
                {
                    var prof = IfcModelBuilder.MakeZShapeProfileDef(m, 150, 90, 13, 20, 6, 3);
                    var face = _xbimGeometryCreator.CreateFace(prof);
                    double area = face.Area;
                    Assert.IsTrue(face.Area > 0);
                    m.ModelFactors.ProfileDefLevelOfDetail = 1;
                    face = _xbimGeometryCreator.CreateFace(prof);
                    //var w = new XbimOccWriter();
                    //w.Write(face, "d:\\xbim\\f");
                    
                    Assert.IsTrue(face.Area > 0);
                    Assert.IsTrue(face.OuterBound.Edges.Count == 12, "Incorrect edge count");
                    Assert.IsTrue(face.Area > area, "Detailed profile should be bigger than normal profile");
                    txn.Commit();
                }
            }
        }
    }
}
