namespace Xbim.Geometry.Abstractions
{
    public interface IXLocation: IXMatrix
    {
        double Scale { get; }
        IXPoint Translation { get;  }
        IXQuaternion Rotation { get; }
        IXLocation Multiplied(IXLocation location);
        IXLocation Inverted();
        IXLocation ScaledBy(double scaleFactor);
        void SetTranslation(double x, double y, double z);
        IXLocation Translated(double x, double y, double z);
    }
}
