using Moq;
using Xbim.Common;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.NetCore.Tests
{
    public static partial class IfcMoq
    {
        
        public static IIfcBlock IfcBlockMoq(IIfcAxis2Placement3D position = null, double xLen = 10, double yLen = 10, double zLen = 10)
        {
            var blockMoq = MakeMoq<IIfcBlock>();
            var block = blockMoq.Object;
            block.XLength = xLen;
            block.YLength = yLen;
            block.ZLength = zLen;
            block.Position = position ?? IfcIfcAxis2Placement3DMock();
            blockMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcBlock)));
            return block;
        }

        

        public static IIfcRectangularPyramid IfcRectangularPyramidMoq(IIfcAxis2Placement3D position = null, double xLen = 10, double yLen = 20, double height = 30)
        {
            var pyramidMoq = MakeMoq<IIfcRectangularPyramid>();          
            var pyramid = pyramidMoq.Object;
            pyramid.XLength = xLen;
            pyramid.YLength = yLen;
            pyramid.Height = height;
            pyramid.Position = position ?? IfcIfcAxis2Placement3DMock();
            pyramidMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcRectangularPyramid)));
            return pyramid;
        }
        public static IIfcRightCircularCone IfcRightCircularConeMoq(IIfcAxis2Placement3D position = null, double height = 30, double radius = 20)
        {
            var coneMoq = MakeMoq<IIfcRightCircularCone>();
            var cone = coneMoq.Object;
            cone.BottomRadius = radius;
            cone.Height = height;           
            cone.Position = position ?? IfcIfcAxis2Placement3DMock();
            coneMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcRightCircularCone)));
            return cone;
        }

        public static IIfcRightCircularCylinder IfcRightCircularCylinderMoq(IIfcAxis2Placement3D position = null, double height = 30, double radius = 20)
        {
            var cylinderMoq = MakeMoq<IIfcRightCircularCylinder>();
            var cylinder = cylinderMoq.Object;
            cylinder.Radius = radius;
            cylinder.Height = height;
            cylinder.Position = position ?? IfcIfcAxis2Placement3DMock();
            cylinderMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcRightCircularCylinder)));
            return cylinder;
        }
        public static IIfcSphere IfcSphereMoq(IIfcAxis2Placement3D position = null, double radius = 20)
        {
            var sphereMoq = MakeMoq<IIfcSphere>();
            var sphere = sphereMoq.Object;
            sphere.Radius = radius;
            sphere.Position = position ?? IfcIfcAxis2Placement3DMock();
            sphereMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcSphere)));
            return sphere;
        }

    }
}
