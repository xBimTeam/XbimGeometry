using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3.MeasureResource;
using Xbim.Ifc4x3.GeometryResource;
using IfcCartesianTransformationOperator3D = Xbim.Ifc4.GeometryResource.IfcCartesianTransformationOperator3D;
using IfcCartesianTransformationOperator3DnonUniform = Xbim.Ifc4.GeometryResource.IfcCartesianTransformationOperator3DnonUniform;

namespace Xbim.Geometry.Engine.Tests
{
    public static partial class IfcMoq
    {
        public static IIfcCartesianTransformationOperator3D IfcCartesianTransformationOperator3DMoq(double scale = 1, double x = 0, double y = 0, double z = 0)
        {
            var ct3dMoq = MakeMoq<IIfcCartesianTransformationOperator3D>();
            ct3dMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcCartesianTransformationOperator3D)));
            ct3dMoq.SetupGet(s => s.Scl).Returns(scale);
            var ct3d = ct3dMoq.Object;
            /*ct3d.Axis1 =  IfcMoq.IfcDirection3dMock(1,0,0); //X dir*/
            /*ct3d.Axis2 = IfcMoq.IfcDirection3dMock(0, 1, 0);*/
            /*ct3d.Axis3 = IfcMoq.IfcDirection3dMock(0, 0, 1);*/
            ct3d.LocalOrigin = IfcMoq.IfcCartesianPoint3dMock(x, y, z);
            ct3d.Scale = scale;
            return ct3d;
        }
        
        public static IfcPointByDistanceExpression IfcPointByDistanceExpressionMoq(double distanceAlongCurve = 2.0)
        {
            var pointExpressionMoq = MakeMoq<IfcPointByDistanceExpression>();
            var circ1 = IfcMoq.IfcCircle3dMock(radius: 20);
            var arc = IfcMoq.IfcTrimmedCurve3dMock(circ1, trimParam2: Math.PI / 2);
            var pointByDistanceExpression = pointExpressionMoq.Object;
            pointByDistanceExpression.BasisCurve = arc as IfcCurve;
            pointByDistanceExpression.DistanceAlong = (IfcLengthMeasure)distanceAlongCurve;
            return pointByDistanceExpression;
        }

        public static IIfcCartesianTransformationOperator3D IfcCartesianTransformationOperator3DMoqNonUniform
                                                                (double scale1 = 1, double scale2 = 1, double scale3 = 1)
        {
            var ct3dMoq = MakeMoq<IIfcCartesianTransformationOperator3DnonUniform>();
            ct3dMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcCartesianTransformationOperator3DnonUniform)));
            ct3dMoq.SetupGet(s => s.Scale).Returns(scale1);
            ct3dMoq.SetupGet(s => s.Scl).Returns(scale1);
            ct3dMoq.SetupGet(s => s.Scale2).Returns(scale2);
            ct3dMoq.SetupGet(s => s.Scl2).Returns(scale2);
            ct3dMoq.SetupGet(s => s.Scale3).Returns(scale3);
            ct3dMoq.SetupGet(s => s.Scl3).Returns(scale3);
            ct3dMoq.SetupGet(s => s.LocalOrigin).Returns(IfcMoq.IfcCartesianPoint3dMock(0, 0, 0));
            ct3dMoq.SetupGet(s => s.Axis1).Returns(IfcMoq.IfcDirection3dMock(1, 0, 0));
            ct3dMoq.SetupGet(s => s.Axis2).Returns(IfcMoq.IfcDirection3dMock(0, 1, 0));
            ct3dMoq.SetupGet(s => s.Axis3).Returns(IfcMoq.IfcDirection3dMock(0, 0, 1));
            var ct3d = ct3dMoq.Object;
            return ct3d;
        }
    }
}
