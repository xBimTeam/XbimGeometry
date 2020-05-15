namespace Xbim.Geometry.Abstractions
{
    public interface IXPlane: IXSurface
    {
        /// <summary>
        /// Origin of the plane
        /// </summary>
        IXPoint Location { get; } 
        /// <summary>
        /// Normal or Z direction of the plane
        /// </summary>
        IXDirection Axis { get; }
        /// <summary>
        /// Direction of the X axis or U coordiante values
        /// </summary>
        IXDirection RefDirection { get; }
    }
}
