using System;

namespace Xbim.Geometry.Abstractions
{
    public interface IXLine
    {
        bool Is3d { get; }
        /// <summary>
        /// The multiplier for a parametric unit, if the parametric length is 2 the actual length is is 2 * ParametricUnit
        /// </summary>
        double ParametricUnit { get; }
        /// <summary>
        /// Start of the line
        /// </summary>
        IXPoint Origin { get; }
        /// <summary>
        /// Direction of the line
        /// </summary>
        IXVector Direction { get; }
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
