namespace Xbim.Geometry.Abstractions
{
    public interface IXBSplineSurface : IXBoundedSurface
    {
        int MaxDegree { get; }
        //double[] UKnots { get; }
        //double[] VKnots { get; }
    }

}
