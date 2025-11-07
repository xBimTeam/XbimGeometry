namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeLayeredMaterialInstance : IXShapeMaterialItem
    {
        IXShapeLayeredMaterial LayeredMaterial { get; set; }
        IXDirection Direction { get; set; }
        bool IsReversed { get; set; }
        double Offset { get; set; }
        double? Extent { get; set; }

    }
}
