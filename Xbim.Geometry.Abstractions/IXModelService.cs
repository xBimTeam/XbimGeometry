namespace Xbim.Geometry.Abstractions
{
    public interface IXModelService
    {
        /// <summary>
        /// The distance between two points less than or equal to which they are determined to be coincidental
        /// </summary>
        double Precision { get; }
        /// <summary>
        /// Model units that make up one meter
        /// </summary>
        double OneMeter { get; }
        /// <summary>
        /// The smallest distance between faces and edges before they are determined to be coincidental, 
        /// it is in millimeters the default = 1.0mm
        /// Mostly only relevant to Boolean operations
        /// </summary>
        double MinimumGap { get; set; }
        /// <summary>
        /// The factor to convert model angles to radians, for a radian unit this = 1 and for a degree unit = PI/180;
        /// </summary>
        double RadianFactor { get; }
    }
}
