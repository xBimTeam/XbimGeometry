using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.NetCore.Tests
{
    public static partial class IfcMoq
    {
        public static IIfcCartesianTransformationOperator3D IfcCartesianTransformationOperator3DMoq(double scale = 3)
        {
            var ct3dMoq = MakeMoq<IIfcCartesianTransformationOperator3D>();
            ct3dMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcCartesianTransformationOperator3D)));
            ct3dMoq.SetupGet(s => s.Scl).Returns(scale);
            var ct3d = ct3dMoq.Object;
            ct3d.Axis1 = IfcMoq.IfcDirection3dMock(1,0,0); //X dir
            ct3d.Axis2 = IfcMoq.IfcDirection3dMock(0, 1, 0);
            ct3d.Axis3 = IfcMoq.IfcDirection3dMock(0, 0, 1);
            ct3d.LocalOrigin = IfcMoq.IfcCartesianPoint3dMock(1, 1, 1);
            ct3d.Scale = scale;
            return ct3d;
        }
    }
}
