using System.Collections.Generic;
using System.IO;

namespace Xbim.Geometry.Abstractions.WexBim
{
    /// <summary>
    /// A geometry model is a single WexBimMesh and one or more WexBim shapes that render it in different styles and transformations
    /// </summary>
    public interface IWexBimGeometryModel
    {
        IWexBimMesh Geometry { get; set; }
        IEnumerable<IWexBimShape> Shapes { get; }

        void AddInstance(IWexBimShapeSingleInstance singleInstance);
        void AddInstances(IEnumerable<IWexBimShapeMultiInstance> instances);
        int MatrixCount();
        int ShapeCount();
        int TriangleCount();
        int VertexCount();
        void WriteToStream(BinaryWriter writer);
    }
}
