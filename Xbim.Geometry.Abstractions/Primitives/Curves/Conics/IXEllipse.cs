namespace Xbim.Geometry.Abstractions
{
    public interface IXEllipse : IXConic
    {
        double MajorRadius { get; }
        double MinorRadius { get; }
    }
}
