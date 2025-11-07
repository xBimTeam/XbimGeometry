namespace Xbim.Geometry.Abstractions
{
    /// <summary>
    /// Effectively Abstract interface for an IXShapeAssembly or IXShapeGeometry
    /// </summary>
    public  interface IXShapeComponent : IXShapeItem
    {
        short IfcType { get; set; }
    }
}
