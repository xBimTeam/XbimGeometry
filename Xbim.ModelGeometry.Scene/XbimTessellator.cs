using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Xbim.Tessellator;
using Xbim.Common.Geometry;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.TopologyResource;
using Xbim.IO;
using Xbim.XbimExtensions.SelectTypes;
using XbimGeometry.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    
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
            return 
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
            var faceSets = new List<IEnumerable<IfcFace>>();
            foreach (var faceSet in faceBasedModel.FbsmFaces)
            {
                faceSets.Add(faceSet.CfsFaces);
            }
            return Mesh(faceSets);  
        }

        

        public IXbimShapeGeometryData Mesh(IfcShellBasedSurfaceModel shellBasedModel)
        {
            return Mesh(shellBasedModel.SbsmBoundary);
        }

        public IXbimShapeGeometryData Mesh(IEnumerable<IfcShell> shellSet)
        {
            var shells = new List<IEnumerable<IfcFace>>();
            foreach (var shell in shellSet)
            {
                var closedShell = shell as IfcClosedShell;
                var openShell = shell as IfcOpenShell;
                if(closedShell!=null) shells.Add(closedShell.CfsFaces);
                else if(openShell!=null) shells.Add(openShell.CfsFaces);
            }
            return Mesh(shells);
        }

        

        public IXbimShapeGeometryData Mesh(IfcConnectedFaceSet connectedFaceSet)
        {
            var faces = new List<IEnumerable<IfcFace>>();
            faces.Add(connectedFaceSet.CfsFaces);
            return Mesh(faces);
        }

        public IXbimShapeGeometryData Mesh(IfcFacetedBrep fBRepModel)
        {
            return Mesh(fBRepModel.Outer);
        }

        public IXbimShapeGeometryData Mesh(IEnumerable<IEnumerable<IfcFace>> facesList)
        {
            if (_geometryType == XbimGeometryType.PolyhedronBinary)
                return MeshPolyhedronBinary(facesList);
            //if (_geometryType == XbimGeometryType.Polyhedron)
            //    return MeshPolyhedronText(facesList);
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
                    var fc = (IfcFace)_model.InstancesLocal[unloadedFace.EntityLabel];
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
                            if (idx == -1) //WE HAVE INSERTED A POINT
                            {
                                faceIndices.Add(cachedVertices.Count);
                                cachedVertices.Add(contourVerts[elements[j]].Position);
                            }
                            else
                                faceIndices.Add(idx);
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

        private IXbimShapeGeometryData MeshPolyhedronBinary(IEnumerable<IEnumerable<IfcFace>> facesList)
        {
            IXbimShapeGeometryData shapeGeometry = new XbimShapeGeometry();
            shapeGeometry.Format = (byte)XbimGeometryType.PolyhedronBinary;
          
            
            
            using (var ms = new MemoryStream(0x4000))
            using (var binaryWriter = new BinaryWriter(ms))
            {
                var faceLists = facesList as IList<IEnumerable<IfcFace>> ?? facesList.ToList();
                var triangulations = new List<XbimTriangulatedMesh>(faceLists.Count);
                foreach (var faceList in faceLists)
                    triangulations.Add(TriangulateFaces(faceList)); 
                

                // Write out header
                uint verticesCount = 0;
                uint triangleCount = 0;
                uint facesCount = 0;
                var boundingBox = XbimRect3D.Empty;
                foreach (var triangulatedMesh in triangulations)
                {
                    verticesCount += triangulatedMesh.VertexCount;
                    triangleCount += triangulatedMesh.TriangleCount;
                    facesCount += (uint)triangulatedMesh.Faces.Count;
                    if (boundingBox.IsEmpty) boundingBox = triangulatedMesh.BoundingBox;
                    else boundingBox.Union(triangulatedMesh.BoundingBox);
                }
                
                binaryWriter.Write((byte)1); //stream format version			
// ReSharper disable once RedundantCast
                binaryWriter.Write((UInt32)verticesCount); //number of vertices
                binaryWriter.Write((UInt32)triangleCount); //number of triangles
               
                foreach (var v in triangulations.SelectMany(t=>t.Vertices))
                {    
                    binaryWriter.Write(v.X);
                    binaryWriter.Write(v.Y);
                    binaryWriter.Write(v.Z);
                }
                shapeGeometry.BoundingBox = boundingBox.ToFloatArray();
                //now write out the faces

                binaryWriter.Write(facesCount);
                uint verticesOffset = 0;
                foreach (var triangulatedMesh in triangulations)
                {
                    foreach (var faceGroup in triangulatedMesh.Faces)
                    {
                        var numTrianglesInFace = faceGroup.Value.Count;
                        binaryWriter.Write(-numTrianglesInFace);
                       // triangulatedMesh.Normals[faceGroup.Key].Write(binaryWriter); //write the normal for the face
                        foreach (var triangle in faceGroup.Value)
                        {
                            WriteIndex(binaryWriter, (uint)triangle[0].StartVertexIndex + verticesOffset, verticesCount);
                            triangle[0].PackedNormal.Write(binaryWriter);
                            WriteIndex(binaryWriter, (uint)triangle[0].NextEdge.StartVertexIndex + verticesOffset, verticesCount);
                            triangle[0].NextEdge.PackedNormal.Write(binaryWriter);
                            WriteIndex(binaryWriter, (uint)triangle[0].NextEdge.NextEdge.StartVertexIndex + verticesOffset,
                                verticesCount);
                            triangle[0].NextEdge.NextEdge.PackedNormal.Write(binaryWriter);
                        }
                    }
                    verticesOffset += triangulatedMesh.VertexCount;
                }
                binaryWriter.Flush();
                shapeGeometry.ShapeData = ms.ToArray();
            }
            return shapeGeometry;
        }

        private XbimTriangulatedMesh TriangulateFaces(IEnumerable<IfcFace> ifcFaces)
        {
            var faceId = 0;
            var enumerable = ifcFaces as IList<IfcFace> ?? ifcFaces.ToList();
            var faceCount = enumerable.Count;
            var triangulatedMesh = new XbimTriangulatedMesh(faceCount);
            foreach (var ifcFace in enumerable)
            {
                var fc = (IfcFace)_model.InstancesLocal[ifcFace.EntityLabel];
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

                    foreach (var p in bound.Orientation ? polyLoop.Polygon : polyLoop.Polygon.Reverse())
                        //add all the points into unique collection
                    {
                        var v = new Vec3(p.X, p.Y, is3D ? p.Z : 0);
                        triangulatedMesh.AddVertex(v, ref contour[i]);
                        i++;
                    }
                    contours.Add(contour);
                }

                if (contours.Any())
                {
                    if (contours.Count == 1 && contours[0].Length == 3) //its a triangle just grab it
                    {
                        triangulatedMesh.AddTriangle(contours[0][0].Data, contours[0][1].Data, contours[0][2].Data, faceId);
                        faceId++;
                    }
                    else if (contours.Count == 1 && contours[0].Length == 4) //its a quad just grab it
                    {
                        triangulatedMesh.AddTriangle(contours[0][0].Data, contours[0][1].Data, contours[0][3].Data, faceId);
                        triangulatedMesh.AddTriangle(contours[0][3].Data, contours[0][1].Data, contours[0][2].Data, faceId);
                        faceId++;
                    }
                    else //it is multi-sided and may have holes
                    {
                        tess.AddContours(contours,false);

                        tess.Tessellate(WindingRule.EvenOdd, ElementType.Polygons, 3);
                        var faceIndices = new List<int>(tess.ElementCount*3);
                        var elements = tess.Elements;
                        var contourVerts = tess.Vertices;
                        for (var j = 0; j < tess.ElementCount*3; j++)
                        {
                            var idx = contourVerts[elements[j]].Data;
                            if (idx < 0) //WE HAVE INSERTED A POINT
                            {
                                //add it to the mesh
                                triangulatedMesh.AddVertex(contourVerts[elements[j]].Position, ref contourVerts[elements[j]]);
                            }
                            faceIndices.Add(contourVerts[elements[j]].Data);
                        }
                        if (faceIndices.Count > 0)
                        {
                            for (var j = 0; j < tess.ElementCount; j++)
                            {
                                var p1 = faceIndices[j*3];
                                var p2 = faceIndices[j*3 + 1];
                                var p3 = faceIndices[j*3 + 2];
                                triangulatedMesh.AddTriangle(p1, p2, p3, faceId);
                            }
                            faceId++;
                        }
                    }
                }
            }

            triangulatedMesh.UnifyFaceOrientation();
            return triangulatedMesh;
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