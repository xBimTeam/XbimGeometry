
using System.IO;

namespace Xbim.Geometry.Abstractions.WexBim
{
    public interface IWexBimHeader
    {
        IXPoint LocalWCS { get; set; }
        int MagicNumber { get; }
        int MatrixCount { get; set; }
        float OneMeter { get; set; }
        int ProductCount { get; set; }
        short RegionCount { get; set; }
        int ShapeCount { get; set; }
        int StyleCount { get; set; }
        int TriangleCount { get; set; }
        byte Version { get; }
        int VertexCount { get; set; }
        void WriteToStream(BinaryWriter writer);
    }
}
