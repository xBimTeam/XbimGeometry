using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Ifc.ProfileResource;
using Xbim.Ifc.GeometryResource;
namespace Xbim.ModelGeometry.TestCases
{
    public static class ProfileResourceExtensions
    {
        static public IfcArbitraryProfileDefWithVoids CreateTestCase(this IfcArbitraryProfileDefWithVoids pf)
        {
            IfcArbitraryProfileDefWithVoids profile = new IfcArbitraryProfileDefWithVoids()
            {
                OuterCurve = new IfcCompositeCurve().CreateOuterCurveTestCase(),
                //InnerCurves= new IfcCompositeCurve().CreateInnerCurveTestCase()
            };
            return profile;
        }

        static public IfcArbitraryClosedProfileDef CreateTestCase(this IfcArbitraryClosedProfileDef pf)
        {
            IfcArbitraryClosedProfileDef profile = new IfcArbitraryClosedProfileDef()
            {
                OuterCurve = new IfcCompositeCurve().CreateOuterCurveTestCase()
            };
            return profile;
        }
    }
}
