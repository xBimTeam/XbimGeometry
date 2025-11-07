namespace Xbim.Geometry.Abstractions
{
    public interface IXWexBimMeshFactory : IXModelScoped
    {
        byte[] CreateWexBimMesh(IXShape shape, out IXAxisAlignedBoundingBox bounds);
        byte[] CreateWexBimMesh(IXShape shape, IXMeshFactors meshFactors, double scale, out IXAxisAlignedBoundingBox bounds);
        byte[] CreateWexBimMesh(IXShape shape, out IXAxisAlignedBoundingBox bounds, out bool hasCurves);
        byte[] CreateWexBimMesh(IXShape shape, IXMeshFactors meshFactors, double scale, out IXAxisAlignedBoundingBox bounds, out bool hasCurves);
        byte[] CreateWexBimMesh(IXShape shape, double tolerance, double linearDeflection, double angularDeflection, double scale, out IXAxisAlignedBoundingBox bounds);
        byte[] CreateWexBimMesh(IXShape shape, double tolerance, double linearDeflection, double angularDeflection, double scale, out IXAxisAlignedBoundingBox bounds, out bool hasCurves);

    }
}
