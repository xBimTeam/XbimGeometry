namespace Xbim.Geometry.Abstractions
{
    public enum Units
    {
        // Default,    // Unit (if available) from the document that is getting imported.
        Foot = 1,       // Feet and Inches in decimal notation.
        Inch,       // Inches in decimal notation.
        Meter,      // Meters as decimal values.
        Decimeter,  // Decimeters as decimal values.
        Centimeter, // Centimeters as decimal values.
        Millimeter, // Millimeters as decimal values.
                    // Custom,     // Custom values as decimal values
    }
    public interface IXGeometricPropertyService : IXModelScoped
    {
        /// <summary>
        /// Returns the length in the required unit 
        /// </summary>
        /// <param name="shape"></param>
        /// <param name="unit"></param>
        /// <returns></returns>
        double Length(IXEdge shape, Units unit = Units.Meter);

        /// <summary>
        /// Returns the length in the required unit 
        /// </summary>
        /// <param name="shape"></param>
        /// <param name="unit"></param>
        /// <returns></returns>
        double Length(IXWire shape, Units unit = Units.Meter);

        /// <summary>
        /// Returns the area in the required unit squared
        /// </summary>
        /// <param name="shape"></param>
        /// <param name="unit"></param>
        /// <returns></returns>
        double Area(IXFace shape, Units unit = Units.Meter);

        double Area(IXWire shape, Units unit = Units.Meter);
        /// <summary>
        /// Returns the area in the required unit squared
        /// </summary>
        /// <param name="shape"></param>
        /// <param name="unit"></param>
        /// <returns></returns>
        double SurfaceArea(IXShell shape, Units unit = Units.Meter);

        /// <summary>
        ///  Returns the volume in the required unit cubed
        /// </summary>
        /// <param name="solid"></param>
        /// <param name="unit"></param>
        /// <returns></returns>
        double Volume(IXSolid solid, Units unit = Units.Meter);

        /// <summary>
        /// For any surface return the normal at the specific position, 
        /// the tolerance for the distance of the point from the face surface is taken from the precision defined in the  model 
        /// </summary>
        /// <param name="face"></param>
        /// <param name="position">the point where the normal is measured</param>
        /// <returns></returns>
        IXDirection NormalAt(IXFace face, IXPoint position);
        /// <summary>
        /// For any surface return the normal at the specific position
        /// </summary>
        /// <param name="face"></param>
        /// <param name="position">the point where the normal is measured</param>
        /// <param name="tolerance">the tolerance for the distance of the point from the surface</param>
        /// <returns></returns>
        IXDirection NormalAt(IXFace face, IXPoint position, double tolerance);
    }
}
