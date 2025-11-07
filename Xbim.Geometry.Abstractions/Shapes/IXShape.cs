using System;
using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXShape : IDisposable
    {
        XShapeType ShapeType { get; }
        //calculates the bounding box of the shape
        IXAxisAlignedBoundingBox Bounds();
        string BrepString();
        bool IsValidShape();
        bool IsClosed { get; }
        bool IsEmptyShape();

        /// <summary>
        /// Creates a mesh on the shape compliant with the mesh factors
        /// </summary>
        /// <param name="meshFactors"> Model specific shape service, alter MeshFactors of the ModelService to change mesh courseness</param>
        /// <returns></returns>
        bool Triangulate(IXMeshFactors meshFactors);
        // int MaterialId { get; set; }
        IXLocation Location { get; }
        IEnumerable<IXFace> AllFaces();
        bool IsEqual(IXShape other);
        int ShapeHashCode();
        XOrientation Orientation { get; }
    }
}
