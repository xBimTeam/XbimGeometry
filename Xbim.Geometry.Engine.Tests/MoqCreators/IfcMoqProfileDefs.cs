using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.ProfileResource;

namespace Xbim.Geometry.Engine.Tests
{
    public static partial class IfcMoq
    {
        public static IIfcRectangleProfileDef IfcRectangleProfileDefMock(double x = 100, double y = 200, IfcProfileTypeEnum profileType = IfcProfileTypeEnum.AREA, IIfcAxis2Placement2D? position = null)
        {
            var rectangleProfileDefMoq = MakeMoq<IIfcRectangleProfileDef>();            
            var rectangleProfileDef = rectangleProfileDefMoq.Object;
            rectangleProfileDefMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcRectangleProfileDef)));
            rectangleProfileDef.ProfileType = profileType;
            rectangleProfileDef.XDim = x;
            rectangleProfileDef.YDim = y;
            rectangleProfileDef.Position = position ?? IfcMoq.IfcAxis2Placement2DMock();
            return rectangleProfileDef;
        }

        public static IIfcCircleProfileDef IfcCircleProfileDefMock(double radius = 100, IfcProfileTypeEnum profileType = IfcProfileTypeEnum.AREA, IIfcAxis2Placement2D? position = null)
        {
            var circleProfileDefMoq = MakeMoq<IIfcCircleProfileDef>();
            var circleProfileDef = circleProfileDefMoq.Object;
            circleProfileDefMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcCircleProfileDef)));
            circleProfileDef.ProfileType = profileType;
            circleProfileDef.Radius = radius;           
            circleProfileDef.Position = position ?? IfcMoq.IfcAxis2Placement2DMock();
            return circleProfileDef;
        }
        public static IIfcCircleHollowProfileDef IfcCircleHollowProfileDefMock(double radius = 100, double wallThickness = 10, IfcProfileTypeEnum profileType = IfcProfileTypeEnum.AREA, IIfcAxis2Placement2D? position = null)
        {
            var circleHollowProfileDefMoq = MakeMoq<IIfcCircleHollowProfileDef>();
            var circleHollowProfileDef = circleHollowProfileDefMoq.Object;
            circleHollowProfileDefMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcCircleHollowProfileDef)));
            circleHollowProfileDef.ProfileType = profileType;
            circleHollowProfileDef.Radius = radius;
            circleHollowProfileDef.WallThickness=wallThickness;
            circleHollowProfileDef.Position = position ?? IfcMoq.IfcAxis2Placement2DMock();
            return circleHollowProfileDef;
        }

        public static IIfcCenterLineProfileDef IfcCenterLineProfileDefMock(IIfcBoundedCurve centreLine, double thickness = 10)
        {
            var centeLineProfileDefMoq = MakeMoq<IIfcCenterLineProfileDef>();
            var centeLineProfileDef = centeLineProfileDefMoq.Object;
            centeLineProfileDefMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcCenterLineProfileDef)));
            centeLineProfileDef.ProfileType = IfcProfileTypeEnum.AREA;
            centeLineProfileDef.Thickness = thickness;
            centeLineProfileDef.Curve = centreLine;
            return centeLineProfileDef;
        }
    }
}
