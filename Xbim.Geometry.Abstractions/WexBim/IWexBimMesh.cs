using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions.WexBim
{
    public interface IWexBimMesh
    {

        int FaceCount { get; }
        IEnumerable<IWexBimMeshFace> Faces { get; }
        int TriangleCount { get; }
        byte Version { get; }
        int VertexCount { get; }
        IEnumerable<IFloat3> Vertices { get; }

        byte[] ToByteArray();
    }
}
