using Moq;
using Xbim.Common;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Tests
{
    public static partial class IfcMoq
    {
        
        public static IIfcBlock IfcBlockMoq(IIfcAxis2Placement3D? position = null, double xLen = 10, double yLen = 10, double zLen = 10)
        {
            var blockMoq = MakeMoq<IIfcBlock>();
            var block = blockMoq.Object;
            block.XLength = xLen;
            block.YLength = yLen;
            block.ZLength = zLen;
            block.Position = position ?? IfcAxis2Placement3DMock();
            blockMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcBlock)));
            return block;
        }

        

        public static IIfcRectangularPyramid IfcRectangularPyramidMoq(IIfcAxis2Placement3D? position = null, double xLen = 10, double yLen = 20, double height = 30)
        {
            var pyramidMoq = MakeMoq<IIfcRectangularPyramid>();          
            var pyramid = pyramidMoq.Object;
            pyramid.XLength = xLen;
            pyramid.YLength = yLen;
            pyramid.Height = height;
            pyramid.Position = position ?? IfcAxis2Placement3DMock();
            pyramidMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcRectangularPyramid)));
            return pyramid;
        }
        public static IIfcRightCircularCone IfcRightCircularConeMoq(IIfcAxis2Placement3D? position = null, double height = 30, double radius = 20)
        {
            var coneMoq = MakeMoq<IIfcRightCircularCone>();
            var cone = coneMoq.Object;
            cone.BottomRadius = radius;
            cone.Height = height;           
            cone.Position = position ?? IfcAxis2Placement3DMock();
            coneMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcRightCircularCone)));
            return cone;
        }

        public static IIfcRightCircularCylinder IfcRightCircularCylinderMoq(IIfcAxis2Placement3D? position = null, double height = 30, double radius = 20)
        {
            var cylinderMoq = MakeMoq<IIfcRightCircularCylinder>();
            var cylinder = cylinderMoq.Object;
            cylinder.Radius = radius;
            cylinder.Height = height;
            cylinder.Position = position ?? IfcAxis2Placement3DMock();
            cylinderMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcRightCircularCylinder)));
            return cylinder;
        }
        public static IIfcSphere IfcSphereMoq(IIfcAxis2Placement3D? position = null, double radius = 20)
        {
            var sphereMoq = MakeMoq<IIfcSphere>();
            var sphere = sphereMoq.Object;
            sphere.Radius = radius;
            sphere.Position = position ?? IfcAxis2Placement3DMock();
            sphereMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcSphere)));
            return sphere;
        }
        public static IIfcSweptDiskSolid IfcSweptDiskSolidMoq(IIfcCurve? directrix = null, double radius = 20, double innerRadius = 10, double? startParam = null, double? endParam = null)
        {
            var sweptMoq = MakeMoq<IIfcSweptDiskSolid>();
            sweptMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var swept = sweptMoq.Object;
            swept.Radius = radius;
            swept.InnerRadius = innerRadius;
            swept.Directrix = directrix ?? IfcMoq.IfcLine3dMock();
            swept.StartParam = startParam;
            swept.EndParam = endParam;
            sweptMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcSweptDiskSolid)));
            return swept;
        }
        public static IIfcExtrudedAreaSolid IfcSweptAreaSolidMoq( IIfcProfileDef? sweptArea = null, double depth = 500, IIfcDirection? extrudeDirection =null, IfcAxis2Placement3D? position = null)
        {
            var sweptMoq = MakeMoq<IIfcExtrudedAreaSolid>();
            sweptMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var swept = sweptMoq.Object;
            swept.SweptArea = sweptArea ?? IfcMoq.IfcRectangleProfileDefMock();
            swept.Position = position ?? IfcMoq.IfcAxis2Placement3DMock();
            swept.Depth = depth;
            swept.ExtrudedDirection = null ?? IfcDirection3dMock();
            sweptMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcExtrudedAreaSolid)));
            return swept;
        }
    }
}
