namespace Xbim.Geometry.Abstractions
{
    public interface IXMatrix 
    {
        bool IsIdentity { get; }
        double M11 { get; }
        double M12 { get; }
        double M13 { get; }
        double ScaleX { get; }
        double M21 { get; }
        double M22 { get; }
        double M23 { get; }
        double ScaleY { get; }
        double M31 { get; }
        double M32 { get; }
        double M33 { get; }
        double ScaleZ { get; }
        double M44 { get; }
        double OffsetX { get; }
        double OffsetY { get; }
        double OffsetZ { get; }
        double[] Values { get; }
        IXMatrix Multiply(IXMatrix matrix);
        byte[] ToByteArray();
    }
}
