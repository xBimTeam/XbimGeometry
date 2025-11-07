namespace Xbim.Geometry.Abstractions
{
    public interface IXLine : IXCurve
    {      
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
        

    }
}
