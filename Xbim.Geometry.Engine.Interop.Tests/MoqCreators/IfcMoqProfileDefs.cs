using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.ProfileResource;

namespace Xbim.Geometry.NetCore.Tests
{
    public static partial class IfcMoq
    {
        public static IIfcRectangleProfileDef IfcRectangleProfileDefMock(double x = 100, double y = 200, IfcProfileTypeEnum profileType = IfcProfileTypeEnum.AREA, IIfcAxis2Placement2D position = null)
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

        public static IIfcCircleProfileDef IfcCircleProfileDefMock(double radius = 100, IfcProfileTypeEnum profileType = IfcProfileTypeEnum.AREA, IIfcAxis2Placement2D position = null)
        {
            var rectangleProfileDefMoq = MakeMoq<IIfcCircleProfileDef>();
            var rectangleProfileDef = rectangleProfileDefMoq.Object;
            rectangleProfileDefMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcCircleProfileDef)));
            rectangleProfileDef.ProfileType = profileType;
            rectangleProfileDef.Radius = radius;           
            rectangleProfileDef.Position = position ?? IfcMoq.IfcAxis2Placement2DMock();
            return rectangleProfileDef;
        }
    }
}
