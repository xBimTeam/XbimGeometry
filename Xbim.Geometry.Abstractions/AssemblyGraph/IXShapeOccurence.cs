namespace Xbim.Geometry.Abstractions
{
    /// <summary>
    /// This is a real instance of a shape, which has colour, position and geometry
    /// </summary>
    public interface IXShapeOccurence
    {
        IXShape Shape { get; set; }
        IXLocation Location { get; set; }
        IXShapeColour Colour { get; set; }
    }
}
