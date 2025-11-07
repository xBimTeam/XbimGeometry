namespace Xbim.Geometry.Abstractions
{
    public interface IXSolid : IXShape
    {
        //Volume in default model units, use IXGeometryPropertyService to get volumes in a specific imperial or metric unit system
        double Volume { get; }
        IXShell[] Shells { get; }
    }
}
