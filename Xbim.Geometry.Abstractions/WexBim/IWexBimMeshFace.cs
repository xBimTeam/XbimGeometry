using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions.WexBim
{
    public interface IWexBimMeshFace
    {
        int ByteSize { get; }
        IEnumerable<int> Indices { get; }
        bool IsPlanar { get; }
        IEnumerable<IFloat3> Normals { get; }
        int OffsetStart { get; }
        int TriangleCount { get; }
        IXDirection NormalAt(int index);
    }
}
