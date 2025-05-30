using System;
using System.Collections.Generic;
using System.Linq;

namespace Xbim.Tessellator.MeshSimplification
{
    /// <summary>
    /// An implementation of Garland-Heckbert simplification algorithm.
    /// The valid vertex pairs valid for contractions are limited to the mesh edges.
    /// </summary>
    public class XbimMeshSimplifier
    {
        private readonly List<Vertex> _vertices = new();
        private readonly XbimTriangulatedMeshConnectivity _connectivity;
        private readonly float _precision;
        private readonly MinHeap _edgeHeap;


        private const float LargeCost = 1e30f;
        private const double NormalFlipThreshold = 0.8;
        private const double MinTriangleArea = 1e-12;

        public XbimMeshSimplifier(XbimTriangulatedMesh mesh, float precision)
        {
            _precision = precision;
            _connectivity = XbimTriangulatedMeshConnectivity.BuildConnectivity(mesh);
            _edgeHeap = new MinHeap(_connectivity.MaxEdgeId);

            ComputeVertexQuadrics(mesh);
            var edges = ComputeAllEdgeCosts().ToList();
            var costs = edges.Select(e => e.Cost).ToArray();
            int[] indicesOfSortedCosts = new int[_connectivity.MaxEdgeId];
            for (int i = 0; i < _connectivity.MaxEdgeId; ++i)
                indicesOfSortedCosts[i] = i;
            Array.Sort((Array)costs, indicesOfSortedCosts);

            foreach (var index in indicesOfSortedCosts)
            {
                var e = _connectivity.GetEdge(index);
                _edgeHeap.Push(e.Id, e.Cost);
            }
        }

        public XbimTriangulatedMesh Simplify(int targetTriCount)
        {
            while (ValidTriangleCount() > targetTriCount && _edgeHeap.Count > 0)
            {
                int ei = _edgeHeap.PopMin();
                var edge = _connectivity.GetEdge(ei);

                if (edge is null || !edge.IsValid)
                    continue;

                if (!CanContract(edge))
                {
                    edge.IsValid = false;
                    continue;
                }

                int keptVertex = ContractEdge(edge);
            }

            var mesh = new XbimTriangulatedMesh(ValidTriangleCount(), _precision);
            var map = new int[_vertices.Count];
            for (int i = 0; i < map.Length; ++i)
            {
                map[i] = -1;
            }

            for (int i = 0; i < _vertices.Count; ++i)
            {
                if (_vertices[i].IsValid)
                    map[i] = mesh.AddVertex(new Vec3(_vertices[i].Position.X, _vertices[i].Position.Y,
                        _vertices[i].Position.Z));
            }

            foreach (var triId in _connectivity.GetTriangleIds())
            {
                var tri = _connectivity.GetTriangle(triId);

                if (tri is null || !tri.IsValid)
                    continue;

                mesh.AddTriangle(map[tri.V0], map[tri.V1], map[tri.V2], tri.FaceId);
            }

            mesh.UnifyFaceOrientation();

            return mesh;
        }


        private void ComputeVertexQuadrics(XbimTriangulatedMesh mesh)
        {
            foreach (var v in mesh.Vertices)
            {
                _vertices.Add(new Vertex { Position = new Vec3(v.X, v.Y, v.Z), IsValid = true, Q = Quadric.Zero });
            }


            foreach (var triId in _connectivity.GetTriangleIds())
            {
                var tri = _connectivity.GetTriangle(triId);

                if (tri is null || !tri.IsValid)
                    continue;

                var p0 = _vertices[tri.V0].Position;
                var p1 = _vertices[tri.V1].Position;
                var p2 = _vertices[tri.V2].Position;

                Vec3.Sub(ref p1, ref p0, out Vec3 e1);
                Vec3.Sub(ref p2, ref p0, out Vec3 e2);
                Vec3.Cross(ref e1, ref e2, out Vec3 normal);
                double len = normal.Length;
                if (len < 1e-12)
                    continue;

                Vec3.Normalize(ref normal);
                var centroid = new Vec3()
                {
                    X = (p0.X + p1.X + p2.X) / 3.0,
                    Y = (p0.Y + p1.Y + p2.Y) / 3.0,
                    Z = (p0.Z + p1.Z + p2.Z) / 3.0,
                };

                var Q = Quadric.FromPlane(normal, centroid);
                tri.Q = Q;

                // area-scaled accumulation 
                var area = 0.5 * len;
                var scaledQ = Q.Scale(area);

                _vertices[tri.V0].Q = _vertices[tri.V0].Q + scaledQ;
                _vertices[tri.V1].Q = _vertices[tri.V1].Q + scaledQ;
                _vertices[tri.V2].Q = _vertices[tri.V2].Q + scaledQ;
            }
        }

        private IEnumerable<Edge> ComputeAllEdgeCosts()
        {
            foreach (var e in _connectivity.GetEdges())
            {
                ComputeEdgeCost(e);
                yield return e;
            }
        }

        private void ComputeEdgeCost(Edge e)
        {
            var v0 = _vertices[e.V0];
            var v1 = _vertices[e.V1];

            if (!v0.IsValid || !v1.IsValid)
            {
                e.IsValid = false;
                e.Cost = LargeCost;
                return;
            }

            Quadric q = v0.Q + v1.Q;
            bool boundaryEdge = _connectivity.IsBoundaryEdge(e.Id);
            bool boundaryV0 = _connectivity.IsBoundaryVertex(e.V0);
            bool boundaryV1 = _connectivity.IsBoundaryVertex(e.V1);

            if (boundaryEdge)
            {
                //e.Cost = LARGE_COST;
                e.Optimal = new Vec3()
                {
                    X = (v0.Position.X + v1.Position.X) * 0.5,
                    Y = (v0.Position.Y + v1.Position.Y) * 0.5,
                    Z = (v0.Position.Z + v1.Position.Z) * 0.5,
                };
                //return;
            }
            else if (boundaryV0)
                e.Optimal = v0.Position;
            else if (boundaryV1)
                e.Optimal = v1.Position;
            else e.Optimal = q.Optimal(v0.Position, v1.Position);

            e.Cost = (float)q.Evaluate(e.Optimal);
        }


        private bool CanContract(Edge e)
        {
            if (e is null || !e.IsValid)
                return false;

            bool boundaryEdge = _connectivity.IsBoundaryEdge(e.Id);
            bool boundaryV0 = _connectivity.IsBoundaryVertex(e.V0);
            bool boundaryV1 = _connectivity.IsBoundaryVertex(e.V1);

            // This happens when you have something like this quad
            //      2
            //     / \
            //    / 1 \
            //   1-----3
            //    \ 2 /
            //     \ /
            //      4
            //
            // The edge 1-3 is not boundary but has its two vertices on the boundary
            // we have to ignore such edges
            if (!boundaryEdge && boundaryV0 && boundaryV1)
                return false;

            // Smell of failure to update connectivity, should not happen at all
            if (boundaryEdge && e.T1 != XbimTriangulatedMeshConnectivity.InvalidId)
                throw new InvalidOperationException("Boundary edge with two triangles");

            var t0 = _connectivity.GetTriangle(e.T0);
            var t0ThirdV = t0.GetThirdVertex(e.V0, e.V1);
            var t1 = _connectivity.GetTriangle(e.T1);
            var t1ThirdV = t1 is null ? XbimTriangulatedMeshConnectivity.InvalidId : t1.GetThirdVertex(e.V0, e.V1);

            if (!t0.IsValid && !t1.IsValid)
                return false;

            if (t0ThirdV == t1ThirdV)
                return false;

            if (!CheckNeighbourhood(e.V0, e.V1, t0ThirdV, t1ThirdV, out int ac, out int ad, out int bc, out int bd))
                return false;

            if (!CheckManifoldness(e.V0, e.V1, t0ThirdV, t1ThirdV, _connectivity.GetVertexEdges(e.V0).Count(),
                    boundaryEdge, ac, e.T0))
                return false;

            if (!CheckNormalFlips(e))
                return false;

            var trisOOfEdge = _connectivity.GetEdgeTriangles(e.Id);
            if (!CheckLinkCondition(e.V0, e.V1, new int[] { trisOOfEdge.T1, trisOOfEdge.T0 }
                    .Select(id => _connectivity.GetTriangle(id)).Where(t => t is not null).ToList()))
                return false;

            return true;
        }


        // True only when every vertex that touches both endpoints of the edge is already part
        // of the two triangles incident to that edge, guaranteeing the operation keeps
        // the mesh 2-manifold and avoids creating a “three-triangle edge
        private bool CheckLinkCondition(int v0, int v1, IReadOnlyList<Triangle> edgeTris)
        {
            // Collect one-ring neighbours of v0 and v1 via connectivity sstructure
            HashSet<int> n0 = new HashSet<int>();
            foreach (int eId in _connectivity.GetVertexEdges(v0))
            {
                var e = _connectivity.GetEdge(eId);
                n0.Add(e.V0 == v0 ? e.V1 : e.V0);
            }

            HashSet<int> n1 = new HashSet<int>();
            foreach (int eId in _connectivity.GetVertexEdges(v1))
            {
                var e = _connectivity.GetEdge(eId);
                n1.Add(e.V0 == v1 ? e.V1 : e.V0);
            }

            n0.Remove(v1);
            n1.Remove(v0);

            n0.IntersectWith(n1); // n0 now holds the intersection

            if (n0.Count == 0) // quick accept – edge is on boundary or simple fan
                return true;

            //vertices already present in the two incident triangles (edgeTris)
            HashSet<int> triVerts = new HashSet<int>();
            foreach (var t in edgeTris)
            {
                if (!t.IsValid) continue;
                triVerts.Add(t.V0);
                triVerts.Add(t.V1);
                triVerts.Add(t.V2);
            }

            // if any common neighbour is not in those two triangles, reject
            foreach (int n in n0)
            {
                if (!triVerts.Contains(n))
                    return false; // contraction would create a 3-triangle edge
            }

            return true;
        }

        /// <summary>
        /// Returns <c>true</c> when collapsing edge (a–b) is guaranteed to keep the surface
        /// 2-manifold. Two situations are guarded against:
        ///   (1) splitting a valence-3 interior vertex,
        ///   (2) zipping up an open boundary flap.
        /// </summary>
        private bool CheckManifoldness(
            int a, int b, // vertices of the edge being contracted
            int c, int d, // the “opposite” vertices (a-c, a-d)
            int valenceA, // degree of vertex a
            bool abIsBoundaryEdge, // IsBoundaryEdge(edgeAB)
            int edgeAC, // edge (a-c) 
            int t0 // one triangle incident to edgeAB
        )
        {
            var edgeBC = XbimTriangulatedMeshConnectivity.InvalidId;

            //----------------------------------------------------------------------
            // 1) Prevent splitting a valence-3 interior vertex
            // ---------------------------------------------------------------------
            // If a has only three incident edges and a-b is interior, then the
            // one-ring around ‘a’ is exactly two triangles {a,b,c} and {a,b,d}.
            // In that configuration the ring can be torn apart when edge (c-d) is
            // also interior *and* its two triangles separate ‘a’ and ‘b’.
            //----------------------------------------------------------------------
            if (valenceA == 3 && !abIsBoundaryEdge)
            {
                int edgeCD = _connectivity.FindEdge(d, c); // internal edge c-d
                if (edgeCD != XbimTriangulatedMeshConnectivity.InvalidId && !_connectivity.IsBoundaryEdge(edgeCD))
                {
                    // the two triangles on c-d
                    (int tEdc0, int tEdc1) = _connectivity.GetEdgeTriangles(edgeCD);

                    //  If one of those triangles contains a and the other contains b, the contraction would create the split
                    bool split =
                        _connectivity.TriangleHasVertex(tEdc0, a) && _connectivity.TriangleHasVertex(tEdc1, b) ||
                        _connectivity.TriangleHasVertex(tEdc0, b) && _connectivity.TriangleHasVertex(tEdc1, a);

                    if (split)
                        return false;
                }
            }

            //----------------------------------------------------------------------
            // 2) Prevent sealing an open boundary flap
            // ---------------------------------------------------------------------
            // If a-b is on the boundary and the other two edges of its only
            // triangle {a,b,c} are *also* on the boundary, collapsing a-b would
            // remove the last open edge of that hole and illegally zip it shut.
            //----------------------------------------------------------------------
            else if (abIsBoundaryEdge && _connectivity.IsBoundaryEdge(edgeAC))
            {
                edgeBC = _connectivity.FindEdgeFromTriangle(b, c, t0); // edge (b-c) inside t0
                if (_connectivity.IsBoundaryEdge(edgeBC))
                    return false;
            }

            return true;
        }


        /// Fails if a and b share any neighbour <c>x</c> that is not c or d
        private bool CheckNeighbourhood(
            int a, int b, // vertices of the edge to contract
            int c, int d, // the two “opposite” vertices (a-c, a-d)
            out int eac, out int ead, // edge-ids (a-c) and (a-d)  (-1 if none)
            out int ebc, out int ebd) // edge-ids (b-c) and (b-d)
        {
            const int InvalidID = XbimTriangulatedMeshConnectivity.InvalidId;

            eac = ead = ebc = ebd = InvalidID;

            // Hash all neighbours of b except a, c, d – we’ll test a’s fan against it.
            HashSet<int> neighboursB = new();

            foreach (int eidB in _connectivity.GetVertexEdges(b))
            {
                int vb = _connectivity.GetOtherVertex(eidB, b);

                if (vb == a) continue; // the edge currently being contracted
                if (vb == c)
                {
                    ebc = eidB;
                    continue;
                }

                if (vb == d)
                {
                    ebd = eidB;
                    continue;
                }

                neighboursB.Add(vb);
            }

            // Check each neighbour of a (except b, c, d) against the hash-set.
            foreach (int eidA in _connectivity.GetVertexEdges(a))
            {
                int va = _connectivity.GetOtherVertex(eidA, a);

                if (va == c)
                {
                    eac = eidA;
                    continue;
                }

                if (va == d)
                {
                    ead = eidA;
                    continue;
                }

                if (va == b) continue;

                if (neighboursB.Contains(va))
                    return false; // shared neighbour
            }

            return true;
        }

        // True when the collapsing won't make 1-ring triangles flip (other than the incident triangles of the contracted edge)
        private bool CheckNormalFlips(Edge removedEdge)
        {
            Vec3 pNew = removedEdge.Optimal;
            // e.T0, e.T1 are gonna be removed
            // so we are checking if any other triangles are gonna be flipped

            foreach (var triId in _connectivity
                         .GetVertexTriangles(removedEdge.V0).Except(new int[] { removedEdge.T0, removedEdge.T1 }))
            {
                var tri = _connectivity.GetTriangle(triId);

                if (tri is null || !tri.IsValid)
                    continue;


                if (tri.V0 != removedEdge.V0 && tri.V1 != removedEdge.V0 && tri.V2 != removedEdge.V0 &&
                    tri.V0 != removedEdge.V1 && tri.V1 != removedEdge.V1 && tri.V2 != removedEdge.V1)
                {
                    continue;
                }

                Vec3 Pos(int vi) => vi == removedEdge.V0 || vi == removedEdge.V1
                    ? pNew
                    : _vertices[vi].Position;

                var p0 = Pos(tri.V0);
                var p1 = Pos(tri.V1);
                var p2 = Pos(tri.V2);

                Vec3.Sub(ref p1, ref p0, out var lhs);
                Vec3.Sub(ref p2, ref p0, out var rhs);
                Vec3.Cross(ref lhs, ref rhs, out var nNew);
                //if (nNew.Length < MIN_TRIANGLE_AREA)
                //    return false;

                var v0Pos = _vertices[tri.V0].Position;
                var v1Pos = _vertices[tri.V1].Position;
                var v2Pos = _vertices[tri.V2].Position;
                Vec3.Sub(ref v1Pos, ref v0Pos, out var lhsOld);
                Vec3.Sub(ref v2Pos, ref v0Pos, out var rhsOld);
                Vec3.Cross(ref lhsOld, ref rhsOld, out var nOld);

                if (nOld.Length < MinTriangleArea)
                    continue;

                Vec3.Normalize(ref nNew);
                Vec3.Normalize(ref nOld);
                Vec3.Dot(ref nNew, ref nOld, out var dot);
                if (dot < NormalFlipThreshold)
                    return false;
            }

            return true;
        }


        private int ContractEdge(Edge edge)
        {
            if (!edge.IsValid)
                return edge.V0;

            int vKeep = edge.V0;
            int vDelete = edge.V1;

            // update position & quadrics 
            _vertices[vKeep].Position = edge.Optimal;
            _vertices[vKeep].Q += _vertices[vDelete].Q;
            _vertices[vDelete].IsValid = false;
            edge.IsValid = false;
            _connectivity.RemoveEdge(edge.Id);

            // cull triangles that referenced vDelete 
            foreach (int tId in _connectivity.GetVertexTriangles(vDelete).ToList())
            {
                if (!_connectivity.ReplaceTriangleVertex(tId, vDelete, vKeep))
                {
                    var tri = _connectivity.GetTriangle(tId);
                    if (tri is not null)
                    {
                        tri.IsValid = false;
                        _connectivity.RemoveTriangle(tId);
                    }
                }
            }

            foreach (int eId in _connectivity.GetVertexEdges(vDelete).ToList())
            {
                var inc = _connectivity.GetEdge(eId);
                if (inc is null || !inc.IsValid)
                    continue;

                _connectivity.ReplaceEdgeVertex(eId, vDelete, vKeep);

                // contracted?
                if (inc.V0 == inc.V1)
                {
                    inc.IsValid = false;
                    _connectivity.RemoveEdge(eId);
                    continue;
                }
            }

            foreach (int eId in _connectivity.GetVertexEdges(vKeep).ToList())
            {
                var e = _connectivity.GetEdge(eId);
                if (!e.IsValid) continue;

                ComputeEdgeCost(e);
                if (_edgeHeap.Contains(eId))
                    _edgeHeap.Update(eId, e.Cost);
                else
                    _edgeHeap.Push(eId, e.Cost);
            }

            return vKeep;
        }

        private int ValidTriangleCount()
        {
            return _connectivity.GetTriangles().Count(t => t is not null && t.IsValid);
        }
    }
}