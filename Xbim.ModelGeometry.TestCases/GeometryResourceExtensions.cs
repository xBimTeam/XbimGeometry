using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Ifc.GeometryResource;
using Xbim.Ifc.Extensions;
namespace Xbim.ModelGeometry.TestCases
{
    public static class GeometryResourceExtensions
    {
        /// <summary>
        /// Creates an outer curve made of two trimmed semi circles
        /// </summary>
        /// <param name="c"></param>
        /// <returns></returns>
        static public IfcCompositeCurve CreateOuterCurveTestCase(this IfcCompositeCurve c)
        {
            IfcCompositeCurve curve = new IfcCompositeCurve()
            {
                
            };
            curve.Segments.Add
                (
                new IfcTrimmedCurve()
                {
                    BasisCurve = new IfcCircle()
                    {
                        Position = new IfcAxis2Placement2D()
                        {
                            Location = new IfcCartesianPoint(0, 0),
                            RefDirection = new IfcDirection(1, 0)
                        }
                    }

                }
                );
            curve.Segments.Add
               (
               new IfcTrimmedCurve()
               {
                   BasisCurve = new IfcCircle()
                   {
                       Position = new IfcAxis2Placement2D()
                       {
                           Location = new IfcCartesianPoint(0, 0),
                           RefDirection = new IfcDirection(1, 0)
                       }
                   }

               }
               );

            return curve;
        }

        static public IfcCompositeCurve CreateInnerCurveTestCase(this IfcCompositeCurve c)
        {
            IfcCompositeCurve curve = new IfcCompositeCurve()
            {

            };
            return curve;
        }

        static public IfcCompositeCurve CreateTestCase(this IfcCompositeCurve c)
        {
            IfcCompositeCurve curve = new IfcCompositeCurve()
            {

            };
            return curve;
        }

        
    }
}
