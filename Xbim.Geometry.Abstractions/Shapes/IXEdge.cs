namespace Xbim.Geometry.Abstractions
{
    public interface IXEdge : IXShape
    {
        //Length in default model units, use IXGeometryPropertyService to get lengths in a specific imperial or metric unit system
        double Length { get; }
        double Tolerance { get; }
        IXCurve EdgeGeometry { get; }
        IXVertex EdgeStart { get; }
        IXVertex EdgeEnd { get; }
    }
}
