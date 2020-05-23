using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.NetCore.Tests
{
    public static partial class IfcMoq
    {
        public static IIfcPolyline IfcPolyline2dMock(double semi1 = 100, double semi2 = 50)
        {
            var polylineMoq = MakeMoq<IIfcPolyline>();
            polylineMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var polyline = polylineMoq.Object;


            polylineMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcPolyline)));
            return polyline;
        }
    }
}
