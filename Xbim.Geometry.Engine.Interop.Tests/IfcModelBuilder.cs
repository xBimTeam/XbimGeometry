using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Common.Step21;
using Xbim.Ifc;
using Xbim.Ifc4.GeometricConstraintResource;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Kernel;
using Xbim.Ifc4.MeasureResource;
using Xbim.Ifc4.ProductExtension;
using Xbim.Ifc4.ProfileResource;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    

    public class IfcModelBuilder
    {
        
        //public static IfcStore CreateandInitModel()
        //{
        //    var credentials = new XbimEditorCredentials
        //    {
        //        ApplicationIdentifier = "Construction Software inc.",
        //        ApplicationFullName = "Ifc sample programme",
        //        ApplicationDevelopersName = "Construction Programmers Ltd",
        //        EditorsOrganisationName = "XbimTeam",
        //        EditorsFamilyName = "Bloggs",
        //        EditorsGivenName = "Jo",
        //        ApplicationVersion = "2.0.1"               
        //    };


        //    var model = IfcStore.Create(credentials,IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel); //create an empty model
            
        //    //Begin a transaction as all changes to a model are transacted
        //    using (var txn = model.BeginTransaction("Initialise Model"))
        //    {              
        //        //set up a project and initialise the defaults
        //        var project = model.Instances.New<IfcProject>();
        //        project.Initialize(ProjectUnits.SIUnitsUK);
        //        model.CalculateModelFactors(); //need to recalculate model factors
        //        project.Name="Test";
        //        //create a building
        //        var building = model.Instances.New<IfcBuilding>();
        //        building.Name = "Building";         
        //        //building.ElevationOfRefHeight = elevHeight;
        //        building.CompositionType = IfcElementCompositionEnum.ELEMENT;

        //        building.ObjectPlacement = model.Instances.New<IfcLocalPlacement>();
        //        var localPlacement = (IfcLocalPlacement) building.ObjectPlacement;

        //        if (localPlacement != null && localPlacement.RelativePlacement == null)
        //            localPlacement.RelativePlacement = model.Instances.New<IfcAxis2Placement3D>();
        //        if (localPlacement != null)
        //        {
        //            var placement = localPlacement.RelativePlacement as IfcAxis2Placement3D;
        //            if (placement != null)
        //            {
        //                placement.Location = model.Instances.New<IfcCartesianPoint>(p=>p.SetXYZ(0.0, 0.0, 0.0)); 
        //            }
        //        }
        //        project.AddBuilding(building);
        //        //commit changes
               
        //        txn.Commit();
        //    }
        //    return model;
        //}

        public static IfcLocalPlacement MakeLocalPlacement(MemoryModel model)
        {
            var objectPlacement = model.Instances.New<IfcLocalPlacement>();
            var relPlacementTo = model.Instances.New<IfcLocalPlacement>();
            var origin = MakeAxis2Placement3D(model);
            relPlacementTo.RelativePlacement = origin;
            objectPlacement.PlacementRelTo = relPlacementTo;
            var displacement = MakeAxis2Placement3D(model);
            objectPlacement.RelativePlacement = displacement;
            return objectPlacement;
        }

        public static IfcExtrudedAreaSolid MakeExtrudedAreaSolid(MemoryModel m, IfcProfileDef profile, double extrude)
        {
            var extrusion = m.Instances.New<IfcExtrudedAreaSolid>();
            extrusion.Depth = extrude;
            extrusion.ExtrudedDirection = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 0, 1));
            extrusion.Position = MakeAxis2Placement3D(m);
            extrusion.SweptArea = profile;
            return extrusion;
        }

        public static IfcRectangleProfileDef MakeRectangleHollowProfileDef(MemoryModel m, double x, double y, double wallThickness)
        {
            var rectProfile = m.Instances.New<IfcRectangleHollowProfileDef>();
            rectProfile.Position = MakeAxis2Placement2D(m);
            rectProfile.XDim = x;
            rectProfile.YDim = y;
            rectProfile.WallThickness = wallThickness;
            return rectProfile;
        }

        public static IfcCircleHollowProfileDef MakeCircleHollowProfileDef(MemoryModel m, double r, double wallThickness)
        {
            var circleProfile = m.Instances.New<IfcCircleHollowProfileDef>();
            circleProfile.Position = MakeAxis2Placement2D(m);
            circleProfile.Radius = r;
            circleProfile.WallThickness = wallThickness;
            return circleProfile;
        }

        public static IfcCircleProfileDef MakeCircleProfileDef(MemoryModel m, double r)
        {
            var circleProfile = m.Instances.New<IfcCircleProfileDef>();
            circleProfile.Position = MakeAxis2Placement2D(m);
            circleProfile.Radius = r;
            return circleProfile;
        }

        public static IfcRectangleProfileDef MakeRectangleProfileDef(MemoryModel m, double x, double y)
        {
            var rectProfile = m.Instances.New<IfcRectangleProfileDef>();
            rectProfile.Position = MakeAxis2Placement2D(m);
            rectProfile.XDim = x;
            rectProfile.YDim = y;
            return rectProfile;
        }

        public static IfcIShapeProfileDef MakeIShapeProfileDef(MemoryModel m, double depth, double width, double flangeThickness, double webThickness, double? filletRadius = null)
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

        public static IfcCartesianTransformationOperator3D MakeCartesianTransformationOperator3D(MemoryModel m)
        {
            var trans = m.Instances.New<IfcCartesianTransformationOperator3D>();
            trans.Axis3 = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 0, 1));
            trans.Axis2 = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 1, 0));
            trans.Axis1 = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
            trans.Scale = 1;
            trans.LocalOrigin = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(0, 0, 0));
            return trans;
        }

        public static IfcCartesianTransformationOperator3DnonUniform MakeCartesianTransformationOperator3DnonUniform(MemoryModel m)
        {
            var trans = m.Instances.New<IfcCartesianTransformationOperator3DnonUniform>();
            trans.Axis3 = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 0, 1));
            trans.Axis2 = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 1, 0));
            trans.Axis1 = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
            trans.Scale = 1;
            trans.Scale = 1;
            trans.Scale = 1;
            trans.LocalOrigin = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(0, 0, 0));
            return trans;
        }

        public static IfcBlock MakeBlock(MemoryModel m, double x, double y, double z)
        {
            var block = m.Instances.New<IfcBlock>();
            block.Position = MakeAxis2Placement3D(m);
            block.XLength = x;
            block.YLength = y;
            block.ZLength = z;
            return block;
        }

        public static IfcRightCircularCylinder MakeRightCircularCylinder(MemoryModel m, double r, double h)
        {
            var cylinder = m.Instances.New<IfcRightCircularCylinder>();
            cylinder.Position = MakeAxis2Placement3D(m);
            cylinder.Radius = r;
            cylinder.Height = h;
            return cylinder;
        }

        public static IfcSphere MakeSphere(MemoryModel m, double r)
        {
            var sphere = m.Instances.New<IfcSphere>();
            sphere.Position = MakeAxis2Placement3D(m);
            sphere.Radius = r;
            return sphere;
        }

        public static IfcPlane MakePlane(MemoryModel m, XbimPoint3D loc, XbimVector3D zdir, XbimVector3D xdir)
        {
            var plane = m.Instances.New<IfcPlane>();
            var p = m.Instances.New<IfcAxis2Placement3D>();
            p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(zdir.X, zdir.Y, zdir.Z));
            p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(loc.X, loc.Y, loc.Z));
            p.RefDirection = m.Instances.New<IfcDirection>(d => d.SetXYZ(xdir.X, xdir.Y, xdir.Z));
            plane.Position = p;
            return plane;
        }

        public static IfcAxis1Placement MakeAxis1Placement(MemoryModel m)
        {
            var p = m.Instances.New<IfcAxis1Placement>();
            p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
            p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));
            return p;
        }
        public static IfcAxis2Placement3D MakeAxis2Placement3D(MemoryModel m)
        {
            var p = m.Instances.New<IfcAxis2Placement3D>();
            p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
            p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));
            return p;
        }

        public static IfcAxis2Placement2D MakeAxis2Placement2D(MemoryModel m)
        {
            var p = m.Instances.New<IfcAxis2Placement2D>();
            p.RefDirection = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
            p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));
            return p;
        }

        public static IfcLShapeProfileDef MakeLShapeProfileDef(MemoryModel m, double depth, double width, double thickness, double? filletRadius = null, double? edgeRadius = null, double? legSlope = null)
        {
            var lProfile = m.Instances.New<IfcLShapeProfileDef>();
            lProfile.Position = MakeAxis2Placement2D(m);
            lProfile.Depth = depth;
            lProfile.Width = width;
            lProfile.Thickness = thickness;
            lProfile.FilletRadius = filletRadius;
            lProfile.EdgeRadius = edgeRadius;
            lProfile.LegSlope = legSlope;
            return lProfile;
        }

        public static IfcUShapeProfileDef MakeUShapeProfileDef(MemoryModel m, double depth, double flangeWidth, double flangeThickness, double webThickness, double? filletRadius = null, double? edgeRadius = null, double? flangeSlope = null)
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
           
            return uProfile;
        }
        public static IfcCShapeProfileDef MakeCShapeProfileDef(MemoryModel m, double depth, double width, double wallThickness, double girth, double? internalFilletRadius = null)
        {
            var cProfile = m.Instances.New<IfcCShapeProfileDef>();
            cProfile.Position = MakeAxis2Placement2D(m);
            cProfile.Depth = depth;
            cProfile.Width = width;
            cProfile.WallThickness = wallThickness;
            cProfile.Girth = girth;
            cProfile.InternalFilletRadius = internalFilletRadius;
            
            return cProfile;
        }

        public static IfcTShapeProfileDef MakeTShapeProfileDef(MemoryModel m, double depth, double flangeWidth, double flangeThickness, double webThickness, double? filletRadius = null, double? flangeEdgeRadius = null, double? webEdgeRadius = null, double? webSlope = null, double? flangeSlope = null)
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
            return tProfile;
        }

        public static IfcZShapeProfileDef MakeZShapeProfileDef(MemoryModel m, double depth, double flangeWidth, double flangeThickness, double webThickness, double? filletRadius = null, double? edgeRadius = null)
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

        public static IfcCenterLineProfileDef MakeCenterLineProfileDef(MemoryModel m, IfcBoundedCurve curve, int thickness)
        {
            var cl = m.Instances.New<IfcCenterLineProfileDef>();
            cl.Thickness = thickness;
            cl.Curve = curve;
            return cl;
        }

        public static IfcTrimmedCurve MakeSemiCircle(MemoryModel m, int radius)
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

        public static IfcArbitraryOpenProfileDef MakeArbitraryOpenProfileDef(MemoryModel m, IfcBoundedCurve curve)
        {
            var def = m.Instances.New<IfcArbitraryOpenProfileDef>();
            def.Curve = curve;
            def.ProfileType=IfcProfileTypeEnum.CURVE;
            return def;
        }

        public static IfcSurfaceOfLinearExtrusion MakeSurfaceOfLinearExtrusion(MemoryModel m, IfcProfileDef profile, double depth, XbimVector3D dir)
        {
            var surf = m.Instances.New<IfcSurfaceOfLinearExtrusion>();
            surf.SweptCurve = profile;
            surf.Depth = depth;
            surf.ExtrudedDirection =  m.Instances.New<IfcDirection>(d=>d.SetXYZ(dir.X,dir.Y,dir.Z));
            return surf;
        }

        public static IfcSurfaceOfRevolution MakeSurfaceOfRevolution(MemoryModel m, IfcProfileDef profile)
        {
            var surf = m.Instances.New<IfcSurfaceOfRevolution>();
            surf.SweptCurve = profile;
            surf.AxisPosition = MakeAxis1Placement(m);
            return surf;
        }

        public static IfcLine MakeLine(MemoryModel m, XbimPoint3D loc, XbimVector3D dir, double len)
        {
            var l = m.Instances.New<IfcLine>();
            l.Dir = m.Instances.New<IfcVector>();
            l.Dir.Magnitude = len;
            l.Dir.Orientation = m.Instances.New<IfcDirection>(d => d.SetXYZ(dir.X, dir.Y, dir.Z));
            l.Pnt = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(loc.X, loc.Y, loc.Z));
            return l;
        }

        public static IfcCompositeCurve MakeCompositeCurve(MemoryModel m)
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

        public static IfcBSplineCurveWithKnots MakeBSplineCurveWithKnots(MemoryModel m)
        {
            var c = m.Instances.New<IfcBSplineCurveWithKnots>();
            
            c.Degree = 3;
            var p1 = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(240, 192, -84));
            var p2 = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(0, 275, -84));
            var p3 = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(-240, 192, -84));
            c.ControlPointsList.Add(p1);
            c.ControlPointsList.Add(p2);
            c.ControlPointsList.Add(p3);
            c.ControlPointsList.Add(m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(0, -108, -84)));
            c.ControlPointsList.Add(p1);
            c.ControlPointsList.Add(p2);
            c.ControlPointsList.Add(p3);

            c.CurveForm = IfcBSplineCurveForm.UNSPECIFIED;
            for (int i = 0; i < 11; i++)
            {
                c.KnotMultiplicities.Add(1);
                c.Knots.Add(i-7.0);
            }

            c.ClosedCurve = false;
            c.SelfIntersect = false;
            c.KnotSpec=IfcKnotType.UNSPECIFIED;
            return c;
        }

        public static IfcRationalBSplineCurveWithKnots MakeRationalBSplineCurveWithKnots(MemoryModel m)
        {
            var c = m.Instances.New<IfcRationalBSplineCurveWithKnots>();

            c.Degree = 3;
            var p1 = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(240, 192, -84));
            var p2 = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(0, 275, -84));
            var p3 = m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(-240, 192, -84));
            c.ControlPointsList.Add(p1);
            c.ControlPointsList.Add(p2);
            c.ControlPointsList.Add(p3);
            c.ControlPointsList.Add(m.Instances.New<IfcCartesianPoint>(p => p.SetXYZ(0, -108, -84)));
            c.ControlPointsList.Add(p1);
            c.ControlPointsList.Add(p2);
            c.ControlPointsList.Add(p3);
            for (int i = 0; i < 7; i++) c.WeightsData.Add(1);
            c.CurveForm = IfcBSplineCurveForm.UNSPECIFIED;
            for (int i = 0; i < 11; i++)
            {
                c.KnotMultiplicities.Add(1);
                c.Knots.Add(i - 7.0);
                
            }

            c.ClosedCurve = false;
            c.SelfIntersect = false;
            c.KnotSpec = IfcKnotType.UNSPECIFIED;
 
            return c;
        }

        public static IfcGridAxis MakeGridAxis(MemoryModel m, string tag, XbimPoint3D start, XbimVector3D dir, double len)
        {
            var gridAxis = m.Instances.New<IfcGridAxis>();
            gridAxis.AxisCurve = MakeLine(m, start, dir,len);
            gridAxis.AxisTag = tag;
            return gridAxis;
        }


        public static IfcGrid MakeGrid(MemoryModel m, int axisCount, double cellSize)
        {
            var grid = m.Instances.New<IfcGrid>();

            for (int i = 0; i < axisCount; i++)
            {
                var u1 = MakeGridAxis(m, "A"+i, new XbimPoint3D(i*cellSize, 0, 0), new XbimVector3D(0, 1, 0), axisCount*cellSize);
                var v1 = MakeGridAxis(m, "a"+i, new XbimPoint3D(0, i * cellSize, 0), new XbimVector3D(1, 0, 0), axisCount * cellSize);
                grid.UAxes.Add(u1);
                grid.VAxes.Add(v1);
            }
            return grid;
        }

    }
}
