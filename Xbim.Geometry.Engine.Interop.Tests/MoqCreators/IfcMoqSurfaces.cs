using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.NetCore.Tests
{
    //Surfaces
    public static partial class IfcMoq
    {
        public static IIfcPlane IfcPlaneMoq(IIfcAxis2Placement3D position = null)
        {
            var planeMoq = MakeMoq<IIfcPlane>();
            planeMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcPlane)));
            var plane = planeMoq.Object;
            plane.Position = position ?? IfcMoq.IfcAxis2Placement3DMock();
            return plane;
        }
    }
}
