using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common;
using Xbim.Common.Step21;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.GeometricConstraintResource;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.Kernel;
using Xbim.Ifc4.ProductExtension;
using Xbim.Ifc4.ProfileResource;
using Xbim.Ifc4.RepresentationResource;
using Xbim.Ifc4.SharedBldgElements;

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
        public void ArbitraryClosedProfileDefWithIncorrectPlacementTest()
        {

            using (var model = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
            {
                using (var txn = model.BeginTransaction("Create Column"))
                {
                    IfcProject project = model.Instances.New<IfcProject>();
                    project.Initialize(ProjectUnits.SIUnitsUK);
                    project.Name = "Test Project";

                    IfcColumn col = model.Instances.New<IfcColumn>();
                    col.Name = "Column With ArbitraryClosedProfileDef";


                    //Creating IfcArbitraryClosedProfileDef that will contain IfcCompositeCurve
                    IfcArbitraryClosedProfileDef arbClosedProf = model.Instances.New<IfcArbitraryClosedProfileDef>();

                    //To create IfcArbitraryClosedProfileDef, we'll need to create IfcCompositeCurve
                    //Creating IfcCompositeCurve
                    IfcCompositeCurve myCompCurve = model.Instances.New<IfcCompositeCurve>();

                    //To Create IfcCompositeCurve, We'll need to create IfcCompositeCurveSegment
                    //create IfcCompositeCurveSegment (polyline)
                    IfcCompositeCurveSegment polylineSeg = model.Instances.New<IfcCompositeCurveSegment>();

                    //create IfcCompositeCurveSegment (arc)
                    IfcCompositeCurveSegment arcSeg = model.Instances.New<IfcCompositeCurveSegment>();

                    //Creating IfcPolyline that will be the parent curve for IfcCompositeCurveSegment "polylineSeg"
                    IfcPolyline myPolyline = model.Instances.New<IfcPolyline>();

                    //Creating Points to build the IfcPolyline
                    IfcCartesianPoint p0 = model.Instances.New<IfcCartesianPoint>();
                    p0.SetXY(200, 100);

                    IfcCartesianPoint p1 = model.Instances.New<IfcCartesianPoint>();
                    p1.SetXY(0, 100);

                    IfcCartesianPoint p2 = model.Instances.New<IfcCartesianPoint>();
                    p2.SetXY(0, 0);

                    IfcCartesianPoint p3 = model.Instances.New<IfcCartesianPoint>();
                    p3.SetXY(400, 0);

                    IfcCartesianPoint p4 = model.Instances.New<IfcCartesianPoint>();
                    p4.SetXY(400, 600);

                    IfcCartesianPoint p5 = model.Instances.New<IfcCartesianPoint>();
                    p5.SetXY(0, 600);

                    IfcCartesianPoint p6 = model.Instances.New<IfcCartesianPoint>();
                    p6.SetXY(0, 500);

                    IfcCartesianPoint p7 = model.Instances.New<IfcCartesianPoint>();
                    p7.SetXY(200, 500);

                    //Adding points to the polyline
                    myPolyline.Points.Add(p0);
                    myPolyline.Points.Add(p1);
                    myPolyline.Points.Add(p2);
                    myPolyline.Points.Add(p3);
                    myPolyline.Points.Add(p4);
                    myPolyline.Points.Add(p5);
                    myPolyline.Points.Add(p6);
                    myPolyline.Points.Add(p7);

                    //Assigning myPolyline to the IfcCompositeCurveSegment polylineSeg
                    polylineSeg.ParentCurve = myPolyline;

                    //Creating Arc using IfcTrimmedCurve
                    IfcTrimmedCurve myArc = model.Instances.New<IfcTrimmedCurve>();

                    //To create IfcTrimmedCurve, We'll need to create IfcCircle and trim it using IfcTrimmingSelect
                    IfcCircle myCirc = model.Instances.New<IfcCircle>();
                    myCirc.Radius = 213.554;
                    IfcCartesianPoint cP = model.Instances.New<IfcCartesianPoint>();
                    cP.SetXY(125.1312, 300); //this should really be a 3D point

                    IfcAxis2Placement3D plcmnt = model.Instances.New<IfcAxis2Placement3D>();
                    plcmnt.Location = cP;
                    plcmnt.RefDirection = model.Instances.New<IfcDirection>();
                    plcmnt.RefDirection.SetXY(0, 1);//this should eb a 3D axis
                    myCirc.Position = plcmnt;

                    myArc.BasisCurve = myCirc;

                    IfcTrimmingSelect v1 = p7;
                    IfcTrimmingSelect v2 = p0;

                    myArc.Trim1.Add(v1);
                    myArc.Trim2.Add(v2);

                    arcSeg.ParentCurve = myArc;

                    //Adding the created two IfcCompositeCurveSegments to the IfcCompositeCurve
                    myCompCurve.Segments.Add(arcSeg);
                    myCompCurve.Segments.Add(polylineSeg);

                    //Assigning IfcCompositeCurve "myCompCurve" to the IfcArbitraryClosedProfileDef
                    arbClosedProf.OuterCurve = myCompCurve;

                    arbClosedProf.ProfileType = IfcProfileTypeEnum.AREA;

                    //model as a swept area solid
                    IfcExtrudedAreaSolid body = model.Instances.New<IfcExtrudedAreaSolid>();
                    body.Depth = 2000;
                    body.SweptArea = arbClosedProf;
                    body.ExtrudedDirection = model.Instances.New<IfcDirection>();
                    body.ExtrudedDirection.SetXYZ(0, 0, 1);

                   
                    txn.Commit();
                    var solid = _xbimGeometryCreator.CreateSolid(body);
                    Assert.IsTrue((int)solid.Volume == 239345450);
                }
            }
        }

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
