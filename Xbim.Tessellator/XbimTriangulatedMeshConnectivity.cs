using System;
using System.Collections.Generic;
using System.Linq;
using Xbim.Tessellator.MeshSimplification;

namespace Xbim.Tessellator
{
    public class XbimTriangulatedMeshConnectivity
    {
        public const int InvalidId = -1;

        private readonly Dictionary<int, Edge> _edges = new();
        private readonly Dictionary<int, Triangle> _triangles = new();
        private readonly Dictionary<(int, int), int> _edgeIndex = new();
        private readonly Dictionary<int, List<int>> _vertexEdges = new();

        private int _nextEdgeId;
        private int _nextTriangleId;

        public int MaxEdgeId => _edges.Count > 0 ? _edges.Keys.Max() + 1 : 0;

        public int MaxTriangleId => _triangles.Count > 0 ? _triangles.Keys.Max() + 1 : 0;

        public bool AllowNonManifold { get; set; } = true;


        public static XbimTriangulatedMeshConnectivity BuildConnectivity(XbimTriangulatedMesh mesh)
        {
            XbimTriangulatedMeshConnectivity connectivity = new XbimTriangulatedMeshConnectivity();
            int nextTriangleId = 0;

            foreach (var face in mesh.Faces)
            {
                int faceId = face.Key;

                foreach (var triangle in face.Value)
                {
                    int triangleId = nextTriangleId++;

                    int v0 = triangle[0].StartVertexIndex;
                    int v1 = triangle[1].StartVertexIndex;
                    int v2 = triangle[2].StartVertexIndex;

                    var added = connectivity.AddTriangle(faceId, v0, v1, v2);

                    if (added == InvalidId)
                        continue;

                    for (int i = 0; i < 3; i++)
                    {
                        XbimTriangleEdge edge = triangle[i];

                        int startVertex = edge.StartVertexIndex;
                        int endVertex = edge.EndVertexIndex;

                        int minVertex = Math.Min(startVertex, endVertex);
                        int maxVertex = Math.Max(startVertex, endVertex);
                        int edgeId = connectivity.FindEdge(minVertex, maxVertex);

                        if (edgeId != InvalidId)
                        {
                            var edgeInfo = connectivity.GetEdge(edgeId);
                            if (edgeInfo.T1 == triangleId || edgeInfo.T0 == triangleId)
                                continue;
                        }

                        connectivity.AddOrAttachEdge(startVertex, endVertex, triangleId);
                    }
                }
            }

            return connectivity;
        }


        public IEnumerable<Edge> GetEdges() => _edges.Values;

        public IEnumerable<int> GetTriangleIds() => _triangles.Keys;
        public IEnumerable<Triangle> GetTriangles() => _triangles.Values;

        public Triangle GetTriangle(int id)
        {
            return _triangles.TryGetValue(id, out var e) ? e : default;

        }

        public int AddTriangle(int faceId, int v0, int v1, int v2)
        {
            if (v0 == v1 || v1 == v2 || v2 == v0)
            {
                return int.MinValue;
            }

            int tId = _nextTriangleId++;
            int e0 = AddOrAttachEdge(v0, v1, tId);
            int e1 = AddOrAttachEdge(v1, v2, tId);
            int e2 = AddOrAttachEdge(v2, v0, tId);
            _triangles[tId] = new Triangle { Id = tId, V0 = v0, V1 = v1, V2 = v2, E0 = e0, E1 = e1, E2 = e2, FaceId = faceId, IsValid = true };
            return tId;
        }

        public int AddTriangle(int v0, int v1, int v2)
        {
            if (v0 == v1 || v1 == v2 || v2 == v0)
                throw new ArgumentException("Degenerate triangle.");

            int tId = _nextTriangleId++;
            int e0 = AddOrAttachEdge(v0, v1, tId);
            int e1 = AddOrAttachEdge(v1, v2, tId);
            int e2 = AddOrAttachEdge(v2, v0, tId);
            _triangles[tId] = new Triangle { Id = tId, V0 = v0, V1 = v1, V2 = v2, E0 = e0, E1 = e1, E2 = e2, IsValid = true };
            return tId;
        }

        public IList<int> RemoveTriangle(int tId)
        {
            if (!_triangles.TryGetValue(tId, out var tri) || tri is null)
                return Array.Empty<int>();

            _triangles[tId] = null;
            var removedEdges = new List<int>();
            foreach (var eId in new[] { tri.E0, tri.E1, tri.E2 })
            {
                if (!_edges.TryGetValue(eId, out var e) || e is null)
                    continue;
                if (e.T0 == tId)
                {
                    e.T0 = e.T1;
                    e.T1 = InvalidId;
                }
                else if (e.T1 == tId)
                    e.T1 = InvalidId;

                if (e.T0 == InvalidId && e.T1 == InvalidId)
                {
                    RemoveEdge(eId);
                    removedEdges.Add(eId);
                }
            }
            return removedEdges;
        }

        public Edge GetEdge(int edgeId)
        {
            return _edges.TryGetValue(edgeId, out var e) ? e : default;
        }

        public int FindEdge(int v0, int v1)
        {
            if (v1 < v0) (v0, v1) = (v1, v0);
            return _edgeIndex.TryGetValue((v0, v1), out var id) ? id : InvalidId;
        }

        public IReadOnlyList<int> GetVertexEdges(int v) =>
            _vertexEdges.TryGetValue(v, out var list) ? list : Array.Empty<int>();

        public (int, int) GetEdgeVertices(int edgeId)
        {
            var e = _edges[edgeId];
            return (e.V0, e.V1);
        }

        public (int, int, int) GetTriangleVertices(int triId)
        {
            var t = _triangles[triId];
            return (t.V0, t.V1, t.V2);
        }

        public bool IsBoundaryEdge(int edgeId) => _edges.TryGetValue(edgeId, out var e) && e is not null && e.IsValid && e.IsBoundary;


        public bool IsBoundaryVertex(int vertexId) =>
            _vertexEdges.TryGetValue(vertexId, out var list) && list.Any(IsBoundaryEdge);


        public IReadOnlyList<int> GetBoundaryVertices() =>
            _vertexEdges.Where(kv => kv.Value.Any(IsBoundaryEdge)).Select(kv => kv.Key).ToArray();


        public int AddOrAttachEdge(int v0, int v1, int triId)
        {
            if (v1 < v0)
                (v0, v1) = (v1, v0);

            if (_edgeIndex.TryGetValue((v0, v1), out var eId))
            {
                var e = _edges[eId];
                if (e.T0 == InvalidId)
                    e.T0 = triId;
                else if (e.T1 == InvalidId)
                    e.T1 = triId;
                else if (!AllowNonManifold)
                    throw new InvalidOperationException("Non-manifold edge");
            }
            else
            {
                eId = _nextEdgeId++;
                _edges[eId] = new Edge(v0, v1, triId, InvalidId) { Id = eId, IsValid = true };
                _edgeIndex[(v0, v1)] = eId;
            }
            RegisterVertexEdge(v0, eId);
            RegisterVertexEdge(v1, eId);
            return eId;
        }

        public IEnumerable<int> GetVertexTriangles(int vertexId)
        {
            foreach (var kv in _triangles)
            {
                var t = kv.Value;
                if (t is null) continue;
                if (t.V0 == vertexId || t.V1 == vertexId || t.V2 == vertexId)
                    yield return kv.Key;
            }
        }

        public bool ReplaceTriangleVertex(int triId, int oldVertex, int newVertex)
        {
            if (!_triangles.TryGetValue(triId, out var tri) || tri is null)
                return false;

            // rewrite vertex indices
            if (tri.V0 == oldVertex) tri.V0 = newVertex;
            if (tri.V1 == oldVertex) tri.V1 = newVertex;
            if (tri.V2 == oldVertex) tri.V2 = newVertex;

            if (tri.IsDegenerate)
            {
                tri.IsValid = false;
                return false;
            }

            if (!tri.IsValid)
                return false;

            // detach the three old edges
            DetachTriangleFromEdge(tri.E0, tri.Id);
            DetachTriangleFromEdge(tri.E1, tri.Id);
            DetachTriangleFromEdge(tri.E2, tri.Id);

            // attach the correct ones
            tri.E0 = AddOrAttachEdge(tri.V0, tri.V1, triId);
            tri.E1 = AddOrAttachEdge(tri.V1, tri.V2, triId);
            tri.E2 = AddOrAttachEdge(tri.V2, tri.V0, triId);

            return true;
        }

        private void DetachTriangleFromEdge(int edgeId, int triId)
        {
            if (!_edges.TryGetValue(edgeId, out var e) || e is null)
                return;

            if (e.T0 == triId) e.T0 = InvalidId;
            else if (e.T1 == triId) e.T1 = InvalidId;
            else return;

            if (e.T0 == InvalidId && e.T1 == InvalidId)
            {
                _edges[edgeId] = null;
                _edgeIndex.Remove((Math.Min(e.V0, e.V1), Math.Max(e.V0, e.V1)));
                UnregisterVertexEdge(e.V0, edgeId);
                UnregisterVertexEdge(e.V1, edgeId);
            }
        }


        public Edge ReplaceEdgeVertex(int edgeId, int oldVertex, int newVertex)
        {
            if (!_edges.TryGetValue(edgeId, out var e) || e is null)
                throw new ArgumentOutOfRangeException(nameof(edgeId));

            // remove edge from oldVertex’s incident list
            UnregisterVertexEdge(oldVertex, edgeId);

            // old canonical key
            _edgeIndex.Remove((Math.Min(e.V0, e.V1), Math.Max(e.V0, e.V1)));

            // swap the vertex
            if (e.V0 == oldVertex) e.V0 = newVertex;
            else if (e.V1 == oldVertex) e.V1 = newVertex;
            else throw new ArgumentException("oldVertex not on edge");

            // canonical order after change
            if (e.V1 < e.V0) (e.V0, e.V1) = (e.V1, e.V0);

            _edges[edgeId] = e;
            _edgeIndex[(e.V0, e.V1)] = edgeId;

            // add edge to newVertex’s incident list
            RegisterVertexEdge(newVertex, edgeId);

            return e;
        }


        private void RegisterVertexEdge(int v, int eId)
        {
            if (!_vertexEdges.TryGetValue(v, out var list))
            {
                list = new List<int>();
                _vertexEdges[v] = list;
            }
            if (!list.Contains(eId)) list.Add(eId);
        }

        private void UnregisterVertexEdge(int v, int eId)
        {
            if (_vertexEdges.TryGetValue(v, out var list))
            {
                list.Remove(eId);
                if (list.Count == 0) _vertexEdges.Remove(v);
            }
        }

        public void RemoveEdge(int eId)
        {
            if (!_edges.TryGetValue(eId, out var e) || e is null)
                return;
            _edges[eId] = null;
            _edgeIndex.Remove((Math.Min(e.V0, e.V1), Math.Max(e.V0, e.V1)));
            foreach (var kv in _triangles)
            {
                var t = kv.Value;
                if (t is null) continue;
                bool changed = false;

                if (t.E0 == eId) { t.E0 = InvalidId; changed = true; }
                else if (t.E1 == eId) { t.E1 = InvalidId; changed = true; }
                else if (t.E2 == eId) { t.E2 = InvalidId; changed = true; }

                if (!changed) continue;

                if (t.E0 == t.E1 && t.E0 != InvalidId || t.E1 == t.E2 && t.E1 != InvalidId || t.E2 == t.E0 && t.E2 != InvalidId)
                {
                    // this triangle has collapsed – mark it invalid or delete it
                    t.IsValid = false;
                    RemoveTriangle(kv.Key);
                    continue;
                }


                if (t.E0 == InvalidId && t.E1 == InvalidId && t.E2 == InvalidId)
                {
                    t.IsValid = false;
                    RemoveTriangle(kv.Key);
                    continue;
                }
            }

            UnregisterVertexEdge(e.V0, eId);
            UnregisterVertexEdge(e.V1, eId);
        }

        internal int GetOtherVertex(int edgeId, int vId)
        {
            if (!_edges.TryGetValue(edgeId, out var e) || e is null)
                throw new ArgumentOutOfRangeException(nameof(edgeId), "Unknown edge ID.");

            if (e.V0 == vId) return e.V1;
            if (e.V1 == vId) return e.V0;

            return InvalidId;
        }


        public (int T0, int T1) GetEdgeTriangles(int edgeId)
        {
            if (!_edges.TryGetValue(edgeId, out var e) || e is null)
                throw new ArgumentOutOfRangeException(nameof(edgeId));

            return (e.T0, e.T1);
        }

        public bool TriangleHasVertex(int triId, int vertexId)
        {
            return _triangles.TryGetValue(triId, out var t) && t is not null &&
                   (t.V0 == vertexId || t.V1 == vertexId || t.V2 == vertexId);
        }

        public int FindEdgeFromTriangle(int v0, int v1, int triId)
        {
            if (v1 < v0)
                (v0, v1) = (v1, v0);

            if (!_edgeIndex.TryGetValue((v0, v1), out var eId))
                return InvalidId;

            var e = _edges[eId];
            return e.T0 == triId || e.T1 == triId ? eId : InvalidId;
        }

        public IReadOnlyList<int> GetNonManifoldEdges()
        {
            // 1.  accumulate how many times each edge appears in the triangle table
            var triCount = new Dictionary<int, int>(_edges.Count);

            foreach (var tri in _triangles.Values)
            {
                if (tri is null || !tri.IsValid) continue;       // skip deleted tris

                void Acc(int eId)
                {
                    if (triCount.TryGetValue(eId, out int n))
                        triCount[eId] = n + 1;
                    else
                        triCount[eId] = 1;
                }

                Acc(tri.E0);
                Acc(tri.E1);
                Acc(tri.E2);
            }

            // 2.  collect edges referenced by > 2 triangles
            return triCount.Where(kv => kv.Value > 2)
                           .Select(kv => kv.Key)
                           .ToArray();
        }

    }
}