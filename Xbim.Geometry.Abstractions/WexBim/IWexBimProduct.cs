using System.IO;

namespace Xbim.Geometry.Abstractions.WexBim
{
    public interface IWexBimProduct
    {
        IXAxisAlignedBoundingBox BoundingBox { get; set; }
        int ProductLabel { get; set; }
        short ProductType { get; set; }

        void WriteToStream(BinaryWriter writer);
    }
}
