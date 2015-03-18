using System;
using System.Collections.Generic;
using System.Linq;
using Xbim.Common.Geometry;
using Xbim.ModelGeometry.Scene;
using Xbim.Tessellator;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimTriangulatedMesh<TGroup>
    {
        public struct  XbimTriangle
        {
            readonly List<Vec3> _vertices;
            readonly XbimTriangleEdge[] _edges;

            internal XbimTriangle(XbimTriangleEdge[] edges, List<Vec3> vertices)
            {
                _vertices = vertices;
                _edges = edges;
            }
           
        }

        private readonly Dictionary<long, XbimTriangleEdge[]> _lookupList;
        private readonly List<XbimTriangleEdge[]> _faultyTriangles = new List<XbimTriangleEdge[]>();
        private readonly Dictionary<TGroup, List<XbimTriangleEdge[]>> _triangleGrouping = new Dictionary<TGroup, List<XbimTriangleEdge[]>>();
        private readonly List<Vec3> _vertices;
        public XbimTriangulatedMesh(List<Vec3> verts, int approxTriangleCount)
        {
            var edgeCount = (int)(approxTriangleCount * 1.5);
            _lookupList = new Dictionary<long, XbimTriangleEdge[]>(edgeCount);
            _triangleGrouping = new Dictionary<TGroup, List<XbimTriangleEdge[]>>(approxTriangleCount);
            _vertices = verts;
        }

        public IEnumerable<XbimTriangle> Triangles
        {
            get 
            {
                return from edgeListList in _triangleGrouping.Values 
                       from edges in edgeListList 
                       select new XbimTriangle(edges, _vertices);
            }
        }
        public List<XbimTriangleEdge[]> FaultyTriangles
        {
            get { return _faultyTriangles; }
        }

        /// <summary>
        /// Returns the normal of the triangle that contains the specified edge
        /// </summary>
        /// <param name="edge"></param>
        /// <returns></returns>
        public XbimPackedNormal TriangleNormal(XbimTriangleEdge edge)
        {
            var p1 = _vertices[edge.StartVertexIndex];
            var p2 = _vertices[edge.NextEdge.StartVertexIndex];
            var p3 = _vertices[edge.NextEdge.NextEdge.StartVertexIndex];    
            var a = new XbimPoint3D(p1.X,p1.Y,p1.Z); 
            var b = new XbimPoint3D(p2.X,p2.Y,p2.Z); 
            var c = new XbimPoint3D(p3.X,p3.Y,p3.Z);
            var cv = XbimVector3D.CrossProduct(b - a, c - a );
            cv.Normalize();
            return new XbimPackedNormal(cv);      
        }

        public Dictionary<TGroup, List<XbimTriangleEdge[]>> TriangleGrouping
        {
            get
            {
                return _triangleGrouping;
            }
        }

       
        private bool AddEdge(XbimTriangleEdge edge)
        {
            var key = edge.Key;
            if (!_lookupList.ContainsKey(key))
            {
                var arr = new XbimTriangleEdge[2];
                arr[0] = edge;
                _lookupList[key] = arr;
            }
            else
            {
                var edges = _lookupList[key];
                if (edges[1] != null)
                    return false; //we already have a pair
                edges[1] = edge;
            }
            return true;
        }

        /// <summary>
        /// Orientates edges to orientate in a uniform direction
        /// </summary>
        /// <returns></returns>
        public void UnifyFaceOrientation()
        {
            foreach (var xbimEdges in _lookupList.Values)
            {
                if (!xbimEdges[0].Frozen)
                    UnifyConnectedTriangles(new []{xbimEdges[0],xbimEdges[0].NextEdge,xbimEdges[0].NextEdge.NextEdge}); //doing the first will do all connected
            }

        }

        private void UnifyConnectedTriangles(XbimTriangleEdge[] startEdges)
        {
            foreach (var edge in startEdges)
            {
                edge.Freeze(); //freeze all the edges in this triangle so we don't switch them twice
                var edgePair = _lookupList[edge.Key];
                XbimTriangleEdge adjacentEdge=null;
                if (edge == edgePair[0]) adjacentEdge = edgePair[1];
                else if(edge == edgePair[1]) adjacentEdge = edgePair[0];
                if (adjacentEdge != null) //if we just have one it is a boundary
                {
                    if (adjacentEdge.EdgeId == edge.EdgeId) //they both face the same way
                    {
                        if (!adjacentEdge.Frozen)
                        {
                            adjacentEdge.Reverse();
                        }
                        else
                            throw new Exception("Invalid triangle orientation");
                    }
                    if (!adjacentEdge.Frozen)
                        UnifyConnectedTriangles(
                            new[] {adjacentEdge.NextEdge, adjacentEdge.NextEdge.NextEdge});

                }
                
            } 
        }

        /// <summary>
        /// Adds the triangle using the three ints as inidices into the vertext collection
        /// </summary>
        /// <param name="p1"></param>
        /// <param name="p2"></param>
        /// <param name="p3"></param>
        /// <param name="grouping"></param>
        public bool AddTriangle(int p1, int p2, int p3, TGroup grouping)
        {
            var e1 = new XbimTriangleEdge(p1);
            var e2 = new XbimTriangleEdge(p2);
            var e3 = new XbimTriangleEdge(p3);
            e1.NextEdge = e2;
            e2.NextEdge = e3;
            e3.NextEdge = e1;
            var triangle = new[] { e1, e2, e3 };
            if (!AddEdge(e1))
            {
                FaultyTriangles.Add(triangle);
                return false;
            }
            if (!AddEdge(e2))
            {
                RemoveEdge(e1);
                FaultyTriangles.Add(triangle);
                return false;
            }
            if (!AddEdge(e3))
            {
                RemoveEdge(e1);
                RemoveEdge(e2);
                FaultyTriangles.Add(triangle);
                return false;
            }
            List<XbimTriangleEdge[]> triangleList;

            if (!_triangleGrouping.TryGetValue(grouping, out triangleList))
            {
                triangleList = new List<XbimTriangleEdge[]>();
                _triangleGrouping.Add(grouping, triangleList);
            }
            triangleList.Add(triangle);
            return true;
        }

        /// <summary>
        /// Removes an edge from the edge list
        /// </summary>
        /// <param name="edge"></param>
        private void RemoveEdge(XbimTriangleEdge edge)
        {
            var edges = _lookupList[edge.Key];
            if (edges[0] == edge) //if it is the first one 
            {
                if (edges[1] == null) //and there is no second one
                    _lookupList.Remove(edge.Key); //remove the entire key
                else
                    edges[0] = edges[1]; //keep the second one
            }
            if (edges[1] == edge) //if it is the second one just remove it and leave the first
                edges[1] = null;
        }
    }

}

/// <summary>
/// Edge class for triangular meshes only
/// </summary>
public class XbimTriangleEdge
{
    public int StartVertexIndex;
    public XbimTriangleEdge NextEdge;
    private bool _frozen;
    public int EndVertexIndex { get { return NextEdge.StartVertexIndex; } }
    public XbimTriangleEdge(int p1)
    {
        StartVertexIndex = p1;
    }

    public bool Frozen
    {
        get { return _frozen; }
        
    }

    public void Freeze()
    {
        _frozen = true;
        NextEdge._frozen=true;
        NextEdge.NextEdge._frozen = true;
    }

    public void Reverse()
    {
        if (!_frozen)
        {
            var p1 = StartVertexIndex;
            var p2 = NextEdge.StartVertexIndex;
            var p3 = NextEdge.NextEdge.StartVertexIndex;
            StartVertexIndex = p2;
            NextEdge.StartVertexIndex = p3;
            NextEdge.NextEdge.StartVertexIndex = p1;
            var prevEdge = NextEdge.NextEdge;
            prevEdge.NextEdge = NextEdge;
            NextEdge.NextEdge = this;
            NextEdge = prevEdge;
        }
    }

    /// <summary>
    /// The ID of the edge, unique for all edges between vertices
    /// </summary>
    public long EdgeId
    {
        get
        {
            long a = StartVertexIndex;
            a <<= 32;
            return (a | EndVertexIndex);
        }
    }

    public bool IsEmpty { get { return EdgeId == 0; } }

    /// <summary>
    /// The key for the edge, this is the same for both directions of an  edge
    /// </summary>
    public long Key
    {
        get
        {
            long left = Math.Max(StartVertexIndex, EndVertexIndex);
            left <<= 32;
            long right = Math.Min(StartVertexIndex, EndVertexIndex);
            return (left | right);
        }
    }

   
}
