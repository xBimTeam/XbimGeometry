namespace Xbim.Geometry.Abstractions
{
    public interface IXVector : IXDirection
    {
        double Magnitude { get; }
    }
}