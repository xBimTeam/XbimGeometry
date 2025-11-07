namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeBinarySerializer
    {
        byte[] ToArray(IXShape shape, bool withTriangles = false, bool withNormals = false);

        IXShape FromArray(byte[] bytes);
    }
}