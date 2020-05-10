using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    public interface IXTrimmedCurve : IXCurve
    {
        IXCurve BasisCurve { get; }
        IXPoint StartPoint { get; } 
        IXPoint EndPoint { get; }
        /// <summary>
        /// Returns the point at parameter uParam, nb considers the parametric unit of the line
        /// </summary>
        /// <param name="uParam"></param>
        /// <returns></returns>
        IXPoint GetPoint(double uParam);
        /// <summary>
        /// Gets the point and the normal at the parameter uParam
        /// </summary>
        /// <param name="uParam"></param>
        /// <param name="normal">Normal at param uParam</param>
        /// <returns></returns>
        IXPoint GetFirstDerivative(double uParam, out IXVector normal);
    }
}
