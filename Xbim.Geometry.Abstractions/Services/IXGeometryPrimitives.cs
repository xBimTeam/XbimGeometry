namespace Xbim.Geometry.Abstractions
{
    public interface IXGeometryPrimitives
    {
        IXPoint BuildPoint3d(double x, double y, double z);
        IXPoint BuildPoint2d(double x, double y);
        IXDirection BuildDirection3d(double x, double y, double z);
        IXDirection BuildDirection2d(double x, double y);
        IXLocation BuildLocation(double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz);
        IXMatrix BuildMatrix(double[] values);

        IXAxisAlignedBoundingBox Moved(IXAxisAlignedBoundingBox box, IXLocation newLocation);
        IXAxisAlignedBoundingBox BuildBoundingBox();
        IXAxisAlignedBoundingBox BuildBoundingBox(double x, double y, double z, double sizeX, double sizeY, double sizeZ );

        IXMeshFactors GetMeshFactors(MeshGranularity granularity, double oneMeter, double precision);
    }
}
