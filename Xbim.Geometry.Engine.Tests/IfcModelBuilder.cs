using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Ifc2x3.Extensions;
using Xbim.Ifc2x3.GeometricConstraintResource;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.Kernel;
using Xbim.Ifc2x3.MeasureResource;
using Xbim.Ifc2x3.ProductExtension;
using Xbim.Ifc2x3.ProfileResource;
using Xbim.IO;

namespace GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    public class IfcModelBuilder
    {
        
        public static XbimModel CreateandInitModel()
        {

            var model = XbimModel.CreateTemporaryModel(); ; //create an empty model

            //Begin a transaction as all changes to a model are transacted
            using (XbimReadWriteTransaction txn = model.BeginTransaction("Initialise Model"))
            {
                //do once only initialisation of model application and editor values
                model.DefaultOwningUser.ThePerson.GivenName = "John";
                model.DefaultOwningUser.ThePerson.FamilyName = "Bloggs";
                model.DefaultOwningUser.TheOrganization.Name = "Department of Building";
                model.DefaultOwningApplication.ApplicationIdentifier = "Construction Software inc.";
                model.DefaultOwningApplication.ApplicationDeveloper.Name = "Construction Programmers Ltd.";
                model.DefaultOwningApplication.ApplicationFullName = "Ifc sample programme";
                model.DefaultOwningApplication.Version = "2.0.1";

                //set up a project and initialise the defaults

                var project = model.Instances.New<IfcProject>();
                project.Initialize(ProjectUnits.SIUnitsUK);
                model.ReloadModelFactors();
                project.Name = "testProject";
                project.OwnerHistory.OwningUser = model.DefaultOwningUser;
                project.OwnerHistory.OwningApplication = model.DefaultOwningApplication;

                //create a building
                var building = model.Instances.New<IfcBuilding>();
                building.Name = "Building";
                building.OwnerHistory.OwningUser = model.DefaultOwningUser;
                building.OwnerHistory.OwningApplication = model.DefaultOwningApplication;
                //building.ElevationOfRefHeight = elevHeight;
                building.CompositionType = IfcElementCompositionEnum.ELEMENT;

                building.ObjectPlacement = model.Instances.New<IfcLocalPlacement>();
                var localPlacement = building.ObjectPlacement as IfcLocalPlacement;

                if (localPlacement != null && localPlacement.RelativePlacement == null)
                    localPlacement.RelativePlacement = model.Instances.New<IfcAxis2Placement3D>();
                if (localPlacement != null)
                {
                    var placement = localPlacement.RelativePlacement as IfcAxis2Placement3D;
                    placement.SetNewLocation(0.0, 0.0, 0.0);
                }

                model.IfcProject.AddBuilding(building);

                //validate and commit changes
                Assert.IsTrue(model.Validate(txn.Modified(), Console.Out) == 0, "Invalid Model");
                txn.Commit();
            }
            return model;
        }

        public static IfcExtrudedAreaSolid MakeExtrudedAreaSolid(XbimModel m, IfcProfileDef profile, double extrude)
        {
            var extrusion = m.Instances.New<IfcExtrudedAreaSolid>();
            extrusion.Depth = extrude;
            extrusion.ExtrudedDirection = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 0, 1));
            extrusion.Position = MakeAxis2Placement3D(m);
            extrusion.SweptArea = profile;
            return extrusion;
        }

        public static IfcRectangleProfileDef MakeRectangleHollowProfileDef(XbimModel m, double x, double y, double wallThickness)
        {
            var rectProfile = m.Instances.New<IfcRectangleHollowProfileDef>();
            rectProfile.Position = MakeAxis2Placement2D(m);
            rectProfile.XDim = x;
            rectProfile.YDim = y;
            rectProfile.WallThickness = wallThickness;
            return rectProfile;
        }

        public static IfcCircleHollowProfileDef MakeCircleHollowProfileDef(XbimModel m, double r, double wallThickness)
        {
            var circleProfile = m.Instances.New<IfcCircleHollowProfileDef>();
            circleProfile.Position = MakeAxis2Placement2D(m);
            circleProfile.Radius = r;
            circleProfile.WallThickness = wallThickness;
            return circleProfile;
        }

        public static IfcCircleProfileDef MakeCircleProfileDef(XbimModel m, double r)
        {
            var circleProfile = m.Instances.New<IfcCircleProfileDef>();
            circleProfile.Position = MakeAxis2Placement2D(m);
            circleProfile.Radius = r;
            return circleProfile;
        }

        public static IfcRectangleProfileDef MakeRectangleProfileDef(XbimModel m, double x, double y)
        {
            var rectProfile = m.Instances.New<IfcRectangleProfileDef>();
            rectProfile.Position = MakeAxis2Placement2D(m);
            rectProfile.XDim = x;
            rectProfile.YDim = y;
            return rectProfile;
        }

        public static IfcIShapeProfileDef MakeIShapeProfileDef(XbimModel m, double depth, double width, double flangeThickness, double webThickness, double? filletRadius = null)
        {
            var iProfile = m.Instances.New<IfcIShapeProfileDef>();
            iProfile.Position = MakeAxis2Placement2D(m);
            iProfile.OverallDepth = depth;
            iProfile.OverallWidth = width;
            iProfile.WebThickness = webThickness;
            iProfile.FlangeThickness = flangeThickness;
            iProfile.FilletRadius = filletRadius;
            return iProfile;
        }

        public static IfcBlock MakeBlock(XbimModel m, double x, double y, double z)
        {
            var block = m.Instances.New<IfcBlock>();
            block.Position = MakeAxis2Placement3D(m);
            block.XLength = x;
            block.YLength = y;
            block.ZLength = z;
            return block;
        }

        public static IfcRightCircularCylinder MakeRightCircularCylinder(XbimModel m, double r, double h)
        {
            var cylinder = m.Instances.New<IfcRightCircularCylinder>();
            cylinder.Position = MakeAxis2Placement3D(m);
            cylinder.Radius = r;
            cylinder.Height = h;
            return cylinder;
        }

        public static IfcSphere MakeSphere(XbimModel m, double r)
        {
            var sphere = m.Instances.New<IfcSphere>();
            sphere.Position = MakeAxis2Placement3D(m);
            sphere.Radius = r;
            return sphere;
        }

        public static IfcPlane MakePlane(XbimModel m, XbimPoint3D loc, XbimVector3D zdir, XbimVector3D xdir)
        {
            var plane = m.Instances.New<IfcPlane>();
            var p = m.Instances.New<IfcAxis2Placement3D>();
            p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(zdir.X, zdir.Y, zdir.Z));
            p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(loc.X, loc.Y, loc.Z));
            p.RefDirection = m.Instances.New<IfcDirection>(d => d.SetXYZ(xdir.X, xdir.Y, xdir.Z));
            plane.Position = p;
            return plane;
        }

        public static IfcAxis1Placement MakeAxis1Placement(XbimModel m)
        {
            var p = m.Instances.New<IfcAxis1Placement>();
            p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
            p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));
            return p;
        }
        public static IfcAxis2Placement3D MakeAxis2Placement3D(XbimModel m)
        {
            var p = m.Instances.New<IfcAxis2Placement3D>();
            p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
            p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));
            return p;
        }

        public static IfcAxis2Placement2D MakeAxis2Placement2D(XbimModel m)
        {
            var p = m.Instances.New<IfcAxis2Placement2D>();
            p.RefDirection = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
            p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));
            return p;
        }

        public static IfcLShapeProfileDef MakeLShapeProfileDef(XbimModel m, double depth, double width, double thickness,  double? filletRadius = null, double? edgeRadius = null, double? legSlope = null, double? centreOfGravityX = null, double? centreOfGravityY = null)
        {
            var lProfile = m.Instances.New<IfcLShapeProfileDef>();
            lProfile.Position = MakeAxis2Placement2D(m);
            lProfile.Depth = depth;
            lProfile.Width = width;
            lProfile.Thickness = thickness;
            lProfile.FilletRadius = filletRadius;
            lProfile.EdgeRadius = edgeRadius;
            lProfile.LegSlope = legSlope;
            lProfile.CentreOfGravityInX = centreOfGravityX;
            lProfile.CentreOfGravityInY = centreOfGravityY;
            return lProfile;
        }

        public static IfcUShapeProfileDef MakeUShapeProfileDef(XbimModel m, double depth, double flangeWidth, double flangeThickness, double webThickness, double? filletRadius = null, double? edgeRadius = null, double? flangeSlope = null, double? centreOfGravityX = null)
        {
            var uProfile = m.Instances.New<IfcUShapeProfileDef>();
            uProfile.Position = MakeAxis2Placement2D(m);
            uProfile.Depth = depth;
            uProfile.FlangeWidth = flangeWidth;
            uProfile.FlangeThickness = flangeThickness;
            uProfile.WebThickness = webThickness;
            uProfile.FilletRadius = filletRadius;
            uProfile.EdgeRadius = edgeRadius;
            uProfile.FlangeSlope = flangeSlope;
            uProfile.CentreOfGravityInX = centreOfGravityX;
            return uProfile;
        }
        public static IfcCShapeProfileDef MakeCShapeProfileDef(XbimModel m, double depth, double width, double wallThickness, double girth, double? internalFilletRadius = null, double? centreOfGravityX = null)
        {
            var cProfile = m.Instances.New<IfcCShapeProfileDef>();
            cProfile.Position = MakeAxis2Placement2D(m);
            cProfile.Depth = depth;
            cProfile.Width = width;
            cProfile.WallThickness = wallThickness;
            cProfile.Girth = girth;
            cProfile.InternalFilletRadius = internalFilletRadius;
            cProfile.CentreOfGravityInX = centreOfGravityX;
            return cProfile;
        }

        public static IfcTShapeProfileDef MakeTShapeProfileDef(XbimModel m, double depth, double flangeWidth, double flangeThickness, double webThickness, double? filletRadius = null, double? flangeEdgeRadius = null, double? webEdgeRadius = null, double? webSlope = null, double? flangeSlope = null, double? centreOfGravityY = null)
        {
            var tProfile = m.Instances.New<IfcTShapeProfileDef>();
            tProfile.Position = MakeAxis2Placement2D(m);
            tProfile.Depth = depth;
            tProfile.FlangeWidth = flangeWidth;
            tProfile.FlangeThickness = flangeThickness;
            tProfile.WebThickness = webThickness;
            tProfile.FilletRadius = filletRadius;
            tProfile.FlangeEdgeRadius = flangeEdgeRadius;
            tProfile.WebEdgeRadius = webEdgeRadius;
            tProfile.WebSlope = webSlope;
            tProfile.FlangeSlope = flangeSlope;
            tProfile.CentreOfGravityInY = centreOfGravityY;
            return tProfile;
        }

        public static IfcZShapeProfileDef MakeZShapeProfileDef(XbimModel m, double depth, double flangeWidth, double flangeThickness, double webThickness, double? filletRadius = null, double? edgeRadius = null)
        {
            var zProfile = m.Instances.New<IfcZShapeProfileDef>();
            zProfile.Position = MakeAxis2Placement2D(m);
            zProfile.Depth = depth;
            zProfile.FlangeWidth = flangeWidth;
            zProfile.FlangeThickness = flangeThickness;
            zProfile.WebThickness = webThickness;
            zProfile.FilletRadius = filletRadius;
            zProfile.EdgeRadius = edgeRadius;
            return zProfile;
        }

        public static IfcCenterLineProfileDef MakeCenterLineProfileDef(XbimModel m, IfcBoundedCurve curve, int thickness)
        {
            var cl = m.Instances.New<IfcCenterLineProfileDef>();
            cl.Thickness = thickness;
            cl.Curve = curve;
            return cl;
        }

        public static IfcTrimmedCurve MakeSemiCircle(XbimModel m, int radius)
        {
            var circle = m.Instances.New<IfcCircle>();
            circle.Position = MakeAxis2Placement2D(m);
            circle.Radius = radius;
            var semiCircle = m.Instances.New<IfcTrimmedCurve>();
            IfcParameterValue t1 = 0.0;
            IfcParameterValue t2 = Math.PI;
            semiCircle.Trim1.Add(t1);
            semiCircle.Trim2.Add(t2);
            semiCircle.MasterRepresentation = IfcTrimmingPreference.PARAMETER;
            semiCircle.BasisCurve = circle;
            return semiCircle;
        }

        public static IfcArbitraryOpenProfileDef MakeArbitraryOpenProfileDef(XbimModel m, IfcBoundedCurve curve)
        {
            var def = m.Instances.New<IfcArbitraryOpenProfileDef>();
            def.Curve = curve;
            def.ProfileType=IfcProfileTypeEnum.CURVE;
            return def;
        }

        public static IfcSurfaceOfLinearExtrusion MakeSurfaceOfLinearExtrusion(XbimModel m, IfcProfileDef profile, double depth, XbimVector3D dir)
        {
            var surf = m.Instances.New<IfcSurfaceOfLinearExtrusion>();
            surf.SweptCurve = profile;
            surf.Depth = depth;
            surf.ExtrudedDirection =  m.Instances.New<IfcDirection>(d=>d.SetXYZ(dir.X,dir.Y,dir.Z));
            return surf;
        }

        public static IfcSurfaceOfRevolution MakeSurfaceOfRevolution(XbimModel m, IfcProfileDef profile)
        {
            var surf = m.Instances.New<IfcSurfaceOfRevolution>();
            surf.SweptCurve = profile;
            surf.AxisPosition = MakeAxis1Placement(m);
            return surf;
        }

        public static IfcLine MakeLine(XbimModel m, XbimPoint3D loc, XbimVector3D dir, double len)
        {
            var l = m.Instances.New<IfcLine>();
            l.Dir = m.Instances.New<IfcVector>();
            l.Dir.Magnitude = len;
            l.Dir.Orientation = m.Instances.New<IfcDirection>(d => d.SetXYZ(dir.X, dir.Y, dir.Z));
            l.Pnt = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(loc.X, loc.Y, loc.Z));
            return l;
        }

        public static IfcCompositeCurve MakeCompositeCurve(XbimModel m)
        {
            var c = m.Instances.New<IfcCompositeCurve>();
            var sc = MakeSemiCircle(m, 10);
            var l = MakeLine(m, new XbimPoint3D(5,0,0),new XbimVector3D(1,0,0),5);
            var s1 = m.Instances.New<IfcCompositeCurveSegment>();
            s1.ParentCurve = sc;
            s1.SameSense = true;
            s1.Transition = IfcTransitionCode.CONTINUOUS;
            c.Segments.Add(s1);
            var s2 = m.Instances.New<IfcCompositeCurveSegment>();
            s2.ParentCurve = l;
            s2.SameSense = true;
            s2.Transition = IfcTransitionCode.CONTINUOUS;
            c.Segments.Add(s2);
            c.SelfIntersect = false;
            return c;
        }

        public static IfcBezierCurve MakeBezierCurve(XbimModel m)
        {
            var c = m.Instances.New<IfcBezierCurve>();
            c.ClosedCurve = false;
            c.Degree = 1;
            c.ControlPointsList.Add(m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(0, 0, 0)));
            c.ControlPointsList.Add(m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(10, 0, 0)));
            c.ControlPointsList.Add(m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(15, 10, 0)));
            c.CurveForm=IfcBSplineCurveForm.POLYLINE_FORM;
            c.SelfIntersect = false;
            return c;
        }

        public static IfcRationalBezierCurve MakeRationalBezierCurve(XbimModel m)
        {
            var c = m.Instances.New<IfcRationalBezierCurve>();
            c.ClosedCurve = false;
            c.Degree = 1;
            c.ControlPointsList.Add(m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(0, 0, 0)));
            c.ControlPointsList.Add(m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(10, 0, 0)));
            c.ControlPointsList.Add(m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(15, 10, 0)));
            c.CurveForm = IfcBSplineCurveForm.POLYLINE_FORM;
            c.SelfIntersect = false;
            c.WeightsData.Add(4);
            c.WeightsData.Add(2);
            c.WeightsData.Add(7);
            return c;
        }
    }
}
