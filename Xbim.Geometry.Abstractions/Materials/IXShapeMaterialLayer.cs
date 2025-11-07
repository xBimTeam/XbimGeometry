namespace Xbim.Geometry.Abstractions
{
    public  interface IXShapeMaterialLayer : IXShapeMaterialItem
    {
        //string Name { get; set; }
        double Thickness { get; set; }
        IXShapeMaterial Material { get; set; }
        int Priority { get; set; }
        bool IsVentilated { get; set; }
    }
}
