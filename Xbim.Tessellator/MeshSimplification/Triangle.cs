using System;
using System.Collections.Generic;

namespace Xbim.Tessellator.MeshSimplification
{
    public class Triangle
    {
        public int Id { get; set; }
        public int V0 { get; set; }
        public int V1 { get; set; }
        public int V2 { get; set; }
        public int E0 { get; set; }
        public int E1 { get; set; }
        public int E2 { get; set; }
        public int FaceId { get; set; }
        public bool IsDegenerate => V0 == V1 || V1 == V2 || V2 == V0;
        public bool IsValid { get; set; }
        public Quadric Q { get; set; }

        internal int GetThirdVertex(int v0, int v1)
        {
            if (V0 == v0 && V1 == v1 || V0 == v1 && V1 == v0)
                return V2;

            if (V1 == v0 && V2 == v1 || V1 == v1 && V2 == v0)
                return V0;

            if (V2 == v0 && V0 == v1 || V2 == v1 && V0 == v0)
                return V1;

            return XbimTriangulatedMeshConnectivity.InvalidId; // not an edge of this tri
        }
    }

    public class TriangleComparer : IEqualityComparer<Triangle>
    {
        public bool Equals(Triangle x, Triangle y)
        {
            var xv = new[] { x.V0, x.V1, x.V2 };
            var yv = new[] { y.V0, y.V1, y.V2 };
            Array.Sort(xv);
            Array.Sort(yv);
            return xv[0] == yv[0] && xv[1] == yv[1] && xv[2] == yv[2];
        }

        public int GetHashCode(Triangle obj)
        {
            int[] v = new[] { obj.V0, obj.V1, obj.V2 };
            Array.Sort(v);
            unchecked
            {
                int hash = 17;
                hash = hash * 31 + v[0];
                hash = hash * 31 + v[1];
                hash = hash * 31 + v[2];
                return hash;
            }
        }
    }
}
