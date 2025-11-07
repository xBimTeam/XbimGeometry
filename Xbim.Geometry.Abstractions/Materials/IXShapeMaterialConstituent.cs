namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeMaterialConstituent : IXShapeMaterialItem
    {
        //string Name { get; set; }
        IXShapeMaterial Material { get; set; }
        double Fraction { get; set; }
    }
}
