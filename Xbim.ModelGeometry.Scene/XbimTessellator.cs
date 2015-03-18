using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.IO.DynamicGrouping;
using Xbim.Tessellator;
using Xbim.Common.Geometry;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.TopologyResource;
using Xbim.IO;
using XbimGeometry.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    struct VertexIndexPairComparer : IEqualityComparer<VertexIndexPair>
    {
        public bool Equals(VertexIndexPair x, VertexIndexPair y)
        {
            return (x.VertexIdxA == y.VertexIdxA && x.VertexIdxB == y.VertexIdxB) ||
            (x.VertexIdxA == y.VertexIdxB && x.VertexIdxB == y.VertexIdxA);
        }

        public int GetHashCode(VertexIndexPair v)
        {          
            long total = v.VertexIdxA + v.VertexIdxB;
            return total.GetHashCode();
        }
    }

    struct VertexIndexPair
    {
        public int VertexIdxA;
        public int VertexIdxB;
        public int FaceIdx;
       
        public VertexIndexPair(int p1, int p2, int f)
        {
            VertexIdxA = p1;
            VertexIdxB = p2;
            FaceIdx = f;
        }

        public bool IsOppositeDirection(VertexIndexPair other)
        {
            return VertexIdxA == other.VertexIdxB && VertexIdxB == other.VertexIdxA;
        }

        public bool IsSameDirection(VertexIndexPair other)
        {
            return VertexIdxA == other.VertexIdxA && VertexIdxA == other.VertexIdxA;
        }
    }

    public class XbimTessellator
    {
        private readonly XbimModel _model;
        private readonly XbimGeometryType _geometryType;
        public XbimTessellator(XbimModel model, XbimGeometryType geometryType)
        {
            _model = model;
            _geometryType = geometryType;
        }

        public IXbimShapeGeometryData Mesh(IXbimGeometryObject geomObject)
        {
            return new XbimShapeGeometry();

        }

        /// <summary>
        /// Returns true if the object can be meshed by the tesselator, if it cannot create an IXbimGeometryObject
        /// </summary>
        /// <param name="shape"></param>
        /// <returns></returns>
        public bool CanMesh(object shape)
        {
            return shape is IXbimGeometryObject ||
                shape is IfcFaceBasedSurfaceModel ||
                shape is IfcShellBasedSurfaceModel ||
                shape is IfcConnectedFaceSet ||
                shape is IfcFacetedBrep;
        }

        public IXbimShapeGeometryData Mesh(IfcRepresentationItem shape)
        {
            var fbm = shape as IfcFaceBasedSurfaceModel;
            if (fbm != null) return Mesh(fbm);
            var sbm = shape as IfcShellBasedSurfaceModel;
            if (sbm != null) return Mesh(sbm);
            var cfs = shape as IfcConnectedFaceSet;
            if (cfs != null) return Mesh(cfs);
            var fbr = shape as IfcFacetedBrep;
            if (fbr != null) return Mesh(fbr);
            throw new ArgumentException("Unsupported representation type for tessellation, " + shape.GetType().Name);
        }


        public IXbimShapeGeometryData Mesh(IfcFaceBasedSurfaceModel faceBasedModel)
        {
            IEnumerable<IfcFace> allFaces = null;
            foreach (var connectFaces in faceBasedModel.FbsmFaces)
            {
                allFaces = allFaces == null ? connectFaces.CfsFaces : allFaces.Concat(connectFaces.CfsFaces);
            }
            return Mesh(allFaces);
        }

        public IXbimShapeGeometryData Mesh(IfcShellBasedSurfaceModel shellBasedModel)
        {
            IEnumerable<IfcFace> allFaces = null;
            foreach (var shell in shellBasedModel.SbsmBoundary)
            {
                var cfs = (IfcConnectedFaceSet)shell;
                allFaces = allFaces == null ? cfs.CfsFaces : allFaces.Concat(cfs.CfsFaces);
            }
            return Mesh(allFaces);
        }

        public IXbimShapeGeometryData Mesh(IfcConnectedFaceSet connectedFaceSet)
        {
            return Mesh(connectedFaceSet.CfsFaces);
        }

        public IXbimShapeGeometryData Mesh(IfcFacetedBrep fBRepModel)
        {
            return Mesh(fBRepModel.Outer);
        }

        public IXbimShapeGeometryData Mesh(IEnumerable<IfcFace> faces)
        {
            if (_geometryType == XbimGeometryType.PolyhedronBinary)
                return MeshPolyhedronBinary(faces);
            if (_geometryType == XbimGeometryType.Polyhedron)
                return MeshPolyhedronText(faces);
            throw new Exception("Illegal Geometry type, " + _geometryType);
        }

        private IXbimShapeGeometryData MeshPolyhedronText(IEnumerable<IfcFace> faces)
        {
            IXbimShapeGeometryData shapeGeometry = new XbimShapeGeometry();
            shapeGeometry.Format = (byte)XbimGeometryType.Polyhedron;
            var numberOfTriangles = 0;
            var unloadedFaces = faces as IList<IfcFace> ?? faces.ToList();
            var faceCount = unloadedFaces.Count;
            
            using (var ms = new MemoryStream(0x4000))
            using (TextWriter textWriter = new StreamWriter(ms))
            {
                var normals = new List<XbimPackedNormal>(faceCount);
                var faceIndicesList = new List<List<int>>(faceCount);
                var verticesIndexLookup = new Dictionary<int, int>();
                var cachedVertices = new List<Vec3>(faceCount*5);

                foreach (var unloadedFace in unloadedFaces)
                {
                    var fc = (IfcFace) _model.Instances[unloadedFace.EntityLabel];
                    //improves performance and reduces memory load
                    var tess = new Tess();
                    var contours = new List<ContourVertex[]>(fc.Bounds.Count);
                    foreach (var bound in fc.Bounds) //build all the loops
                    {
                        var polyLoop = bound.Bound as IfcPolyLoop;
                        if (polyLoop == null || polyLoop.Polygon.Count < 3) continue; //skip non-polygonal faces
                        var is3D = (polyLoop.Polygon[0].Dim == 3);
                        var contour = new ContourVertex[polyLoop.Polygon.Count];
                        var i = 0;
                        foreach (var p in polyLoop.Polygon) //add all the points into unique collection
                        {
                            int index;
                            if (!verticesIndexLookup.TryGetValue(p.EntityLabel, out index))
                            {
                                index = cachedVertices.Count;
                                verticesIndexLookup.Add(p.EntityLabel, index);

                                contour[i].Position.X = (float) p.X;
                                contour[i].Position.Y = (float) p.Y;
                                contour[i].Position.Z = is3D ? (float) p.Z : 0;
                                cachedVertices.Add(contour[i].Position);
                            }
                            else
                                contour[i].Position = cachedVertices[index];
                            contour[i].Data = index;
                            i++;
                        }
                        contours.Add(contour);
                    }
                    if (contours.Any())
                    {
                        tess.AddContours(contours);
                        tess.Tessellate(WindingRule.EvenOdd, ElementType.Polygons, 3);
                        numberOfTriangles += tess.ElementCount;
                        var faceIndices = new List<int>(tess.ElementCount*3);
                        var elements = tess.Elements;
                        var contourVerts = tess.Vertices;
                        for (var j = 0; j < tess.ElementCount*3; j++)
                        {
                            var idx = contourVerts[elements[j]].Data;
                            if (idx == null) //WE HAVE INSERTED A POINT
                            {
                                faceIndices.Add(cachedVertices.Count);
                                cachedVertices.Add(contourVerts[elements[j]].Position);
                            }
                            else
                                faceIndices.Add((int) idx);
                        }
                        if (faceIndices.Count > 0)
                        {
                            faceIndicesList.Add(faceIndices);
                            var n = new XbimVector3D(tess.Normal.X, tess.Normal.Y, tess.Normal.Z);
                            n.Normalize();
                            normals.Add(new XbimPackedNormal(n));
                        }
                    }
                }

                // Write out header
                var verticesCount = (uint) cachedVertices.Count;
                textWriter.WriteLine("P {0} {1} {2} {3} {4}", 1, verticesCount, faceIndicesList.Count, numberOfTriangles,
                    normals.Count);
                //write out vertices and normals  
                textWriter.Write("V");
                var minX = double.PositiveInfinity;
                var minY = minX;
                var minZ = minX;
                var maxX = double.NegativeInfinity;
                var maxY = maxX;
                var maxZ = maxX;
                foreach (var p in cachedVertices)
                {
                    textWriter.Write(" {0},{1},{2}", p.X, p.Y, p.Z);
                    minX = Math.Min(minX, p.X);
                    minY = Math.Min(minY, p.Y);
                    minZ = Math.Min(minZ, p.Z);
                    maxX = Math.Max(maxX, p.X);
                    maxY = Math.Max(maxY, p.Y);
                    maxZ = Math.Max(maxZ, p.Z);
                }
                textWriter.WriteLine();
                textWriter.Write("N");
                foreach (var packedNormal in normals)
                {
                    var n = packedNormal.Normal;
                    textWriter.Write(" {0},{1},{2}", n.X, n.Y, n.Z);
                }
                textWriter.WriteLine();

                //now write out the faces
                var faceIndex = 0;
                foreach (var faceIndices in faceIndicesList)
                {
                    textWriter.Write("T");
                    for (int j = 0; j < faceIndices.Count/3; j++)
                    {
                        if (j == 0)
                            textWriter.Write(" {0}/{3},{1},{2}", faceIndices[j*3], faceIndices[j*3 + 1], faceIndices[j*3 + 2], faceIndex);
                        else
                            textWriter.Write(" {0},{1},{2}", faceIndices[j*3], faceIndices[j*3 + 1], faceIndices[j*3 + 2]);
                    }
                    faceIndex++;
                    textWriter.WriteLine();
                    shapeGeometry.BoundingBox =
                        new XbimRect3D(minX, minY, minZ, maxX - minX, maxY - minY, maxZ - minZ).ToFloatArray();
                   
                } 
                textWriter.Flush();
                shapeGeometry.ShapeData = ms.ToArray();
            }
            return shapeGeometry;
        }

        private IXbimShapeGeometryData MeshPolyhedronBinary(IEnumerable<IfcFace> faces)
        {
            IXbimShapeGeometryData shapeGeometry = new XbimShapeGeometry();
            shapeGeometry.Format = (byte)XbimGeometryType.PolyhedronBinary;
            var numberOfTriangles = 0;
            var unloadedFaces = faces as IList<IfcFace> ?? faces.ToList();
            var faceCount = unloadedFaces.Count;
            using (var ms = new MemoryStream(0x4000))
            using (var binaryWriter = new BinaryWriter(ms))
            {               
                var verticesIndexLookup = new Dictionary<Vec3, int>(new Vec3EqualityComparer());
                var cachedVertices = new List<Vec3>(faceCount * 5);
                var triangulatedMesh = new XbimTriangulatedMesh<int>(cachedVertices, faceCount*2);
                var faceId = 0;
                foreach (var unloadedFace in unloadedFaces)
                {
                    var fc = (IfcFace)_model.Instances[unloadedFace.EntityLabel];
                    //improves performance and reduces memory load
                    var tess = new Tess();
                    var contours = new List<ContourVertex[]>(fc.Bounds.Count);
                    foreach (var bound in fc.Bounds) //build all the loops
                    {
                        var polyLoop = bound.Bound as IfcPolyLoop;
                       
                        if (polyLoop == null || polyLoop.Polygon.Count < 3) continue; //skip non-polygonal faces

                        var is3D = (polyLoop.Polygon[0].Dim == 3);
                        var contour = new ContourVertex[polyLoop.Polygon.Count];
                        var i = 0;
                        
                        foreach (var p in bound.Orientation?polyLoop.Polygon:polyLoop.Polygon.Reverse()) //add all the points into unique collection
                        {
                            int index;
                            var v = new Vec3(p.X, p.Y, is3D ? p.Z : 0);
                            if (!verticesIndexLookup.TryGetValue(v, out index))
                            {
                                index = cachedVertices.Count;
                                verticesIndexLookup.Add(v, index);
                                contour[i].Position = v;
                                cachedVertices.Add(v);
                            }
                            else
                                contour[i].Position = cachedVertices[index];
                            contour[i].Data = index;
                            i++;
                        }
                        contours.Add(contour);
                    }

                    if (contours.Any())
                    {
                        tess.AddContours(contours);
                        tess.Tessellate(WindingRule.EvenOdd, ElementType.Polygons, 3);
                        numberOfTriangles += tess.ElementCount;
                        var faceIndices = new List<int>(tess.ElementCount * 3);
                        var elements = tess.Elements;
                        var contourVerts = tess.Vertices;
                        for (var j = 0; j < tess.ElementCount * 3; j++)
                        {
                            var idx = contourVerts[elements[j]].Data;
                            if (idx == null) //WE HAVE INSERTED A POINT
                            {
                                idx = cachedVertices.Count;
                                cachedVertices.Add(contourVerts[elements[j]].Position);
                                contourVerts[elements[j]].Data = idx;
                            }
                            faceIndices.Add((int)idx);
                        }
                        if (faceIndices.Count > 0)
                        {
                            for (var j = 0; j < tess.ElementCount; j++)
                            {
                                var p1 = faceIndices[j * 3];
                                var p2 = faceIndices[j * 3 + 1];
                                var p3 = faceIndices[j * 3 + 2];
                                triangulatedMesh.AddTriangle(p1, p2, p3, faceId);
                            }
                            faceId++;
                        }
                    }
                }

                triangulatedMesh.UnifyFaceOrientation();
               
                // Write out header
                var verticesCount = (uint)cachedVertices.Count;
                binaryWriter.Write((byte)1); //stream format version			
// ReSharper disable once RedundantCast
                binaryWriter.Write((UInt32)verticesCount); //number of vertices
                binaryWriter.Write((UInt32)numberOfTriangles); //number of triangles
                var minX = double.PositiveInfinity;
                var minY = minX;
                var minZ = minX;
                var maxX = double.NegativeInfinity;
                var maxY = maxX;
                var maxZ = maxX;
                foreach (var cachedVertex in cachedVertices)
                {
                    binaryWriter.Write(cachedVertex.X);
                    binaryWriter.Write(cachedVertex.Y);
                    binaryWriter.Write(cachedVertex.Z);
                    minX = Math.Min(minX, cachedVertex.X);
                    minY = Math.Min(minY, cachedVertex.Y);
                    minZ = Math.Min(minZ, cachedVertex.Z);
                    maxX = Math.Max(maxX, cachedVertex.X);
                    maxY = Math.Max(maxY, cachedVertex.Y);
                    maxZ = Math.Max(maxZ, cachedVertex.Z);
                }
                shapeGeometry.BoundingBox =
                    new XbimRect3D(minX, minY, minZ, maxX - minX, maxY - minY, maxZ - minZ).ToFloatArray();
                //now write out the faces
                var faceIndex = 0;

                binaryWriter.Write((Int32)triangulatedMesh.TriangleGrouping.Count);
                foreach (var faceGroup in triangulatedMesh.TriangleGrouping)
                {
                    var numTrianglesInFace = faceGroup.Value.Count;
                    if (numTrianglesInFace > 0)
                    {
                        var norm = triangulatedMesh.TriangleNormal(faceGroup.Value[0][0]);
                        //XbimPackedNormal norm;//default(XbimPackedNormal);
                        //if (!float.IsNaN(n.X))
                        //    //if it is a line use any valid normal, the default will do, added when the array was created
                        //    norm = new XbimPackedNormal(n.X, n.Y, n.Z);
                        //else
                        //    norm = new XbimPackedNormal(0, 1, 0);
                        binaryWriter.Write((Int32)numTrianglesInFace);
                        norm.Write(binaryWriter); //write the normal for the face
                        foreach (var triangle in faceGroup.Value)
                        {
                            WriteIndex(binaryWriter, (uint)triangle[0].StartVertexIndex, verticesCount);
                            WriteIndex(binaryWriter, (uint)triangle[1].StartVertexIndex, verticesCount);
                            WriteIndex(binaryWriter, (uint)triangle[2].StartVertexIndex, verticesCount);
                        }
                    }
                }


                //// ReSharper disable once RedundantCast
                //binaryWriter.Write((Int32)faceIndicesList.Count);

                //foreach (var faceIndices in faceIndicesList)
                //{
                //    var numTrianglesInFace = faceIndices.Count / 3;
                //    var norm = normals[faceIndex];
                //    faceIndex++;
                //    // ReSharper disable once RedundantCast
                //    binaryWriter.Write((Int32)numTrianglesInFace);
                //    norm.Write(binaryWriter); //write the normal for the face
                //    foreach (var fi in faceIndices)
                //        WriteIndex(binaryWriter, (uint)fi, verticesCount);
                //}
                binaryWriter.Flush();
                shapeGeometry.ShapeData = ms.ToArray();
            }
            return shapeGeometry;
        }


        private void WriteIndex(BinaryWriter bw, UInt32 index, UInt32 maxInt)
        {
            if (maxInt <= 0xFF)
                bw.Write((byte)index);
            else if (maxInt <= 0xFFFF)
                bw.Write((UInt16)index);
            else
                bw.Write(index);
        }

       
    }
}