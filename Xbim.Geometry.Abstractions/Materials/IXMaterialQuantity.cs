namespace Xbim.Geometry.Abstractions
{
    public interface IXMaterialQuantity
    {
        int IfcId { get; set; }
        string IfcType { get; set; }
        string Classification { get; set; }
        double? Volume { get; set; }
        double? Area { get; set; }
        double? Thickness { get; set; }
        string Material { get; set; }
    }
}
