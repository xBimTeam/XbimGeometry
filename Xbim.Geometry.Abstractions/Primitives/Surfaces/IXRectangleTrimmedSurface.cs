namespace Xbim.Geometry.Abstractions
{
    public interface IXRectangleTrimmedSurface : IXBoundedSurface
    {
        IXSurface BasisSurface { get; }
        void Bounds(ref double u1 , ref double u2, ref double v1, ref double v2); 
    }
}
