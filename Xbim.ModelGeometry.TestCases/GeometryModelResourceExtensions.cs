using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.ModelGeometry;
using Xbim.Ifc.GeometricModelResource;
using Xbim.Ifc.ProfileResource;
using Xbim.XbimExtensions;

namespace Xbim.ModelGeometry.TestCases
{
    static public class GeometryModelResourceExtensions
    {
        static public IfcSurfaceCurveSweptAreaSolid CreateTestCase(this IfcSurfaceCurveSweptAreaSolid sweep)
        {
            IfcSurfaceCurveSweptAreaSolid sweptAreaSolid = new IfcSurfaceCurveSweptAreaSolid()
                {
                    SweptArea = new IfcArbitraryProfileDefWithVoids().CreateTestCase()
                };

            return sweptAreaSolid;
        }
    }
}
