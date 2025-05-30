using System.Collections.Generic;
using Xbim.Tessellator;

namespace Xbim.Tessellator.MeshSimplification
{
    public class Edge
    {

        public Edge(int v0, int v1, int t0, int t1)
        {
            V0 = v0;
            V1 = v1;
            T0 = t0;
            T1 = t1;
        }

        public int Id { get; set; }
        public int V0 { get; set; }
        public int V1 { get; set; }
        public int T0 { get; set; }
        public int T1 { get; set; }

        public float Cost { get; set; }
        public Vec3 Optimal { get; set; }
        public bool IsValid { get; set; }

        public bool IsBoundary => T0 == XbimTriangulatedMeshConnectivity.InvalidId || T1 == XbimTriangulatedMeshConnectivity.InvalidId;
        public bool IsNonManifold => T0 != XbimTriangulatedMeshConnectivity.InvalidId && T1 != XbimTriangulatedMeshConnectivity.InvalidId && T0 == T1;
    }

    internal class EdgeComparer : IEqualityComparer<Edge>
    {
        public bool Equals(Edge x, Edge y)
        {
            return x.V0 == y.V0 && x.V1 == y.V1 || x.V0 == y.V1 && x.V1 == y.V0;
        }

        public int GetHashCode(Edge obj)
        {
            int[] v = new[] { obj.V0, obj.V1 };
            System.Array.Sort(v);
            unchecked
            {
                int hash = 17;
                hash = hash * 31 + v[0];
                hash = hash * 31 + v[1];
                return hash;
            }
        }
    }
}
