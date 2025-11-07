using System.Collections.Generic;
using System.IO;

namespace Xbim.Geometry.Abstractions.WexBim
{
    public interface IWexBimRegion
    {
        float CentreX { get; set; }
        float CentreY { get; set; }
        float CentreZ { get; set; }
        IList<IWexBimGeometryModel> GeometryModels { get; }
        int Population { get; set; }

        void AddGeometryModel(byte[] triangulation, IEnumerable<IWexBimShapeMultiInstance> instances);
        void AddGeometryModel(byte[] triangulation, IWexBimShapeSingleInstance singleInstance);
        void AddGeometryModel(IWexBimGeometryModel gm);
        int MatrixCount();
        int ShapeCount();
        int TriangleCount();
        int VertexCount();
        void WriteToStream(BinaryWriter writer);
    }
}
