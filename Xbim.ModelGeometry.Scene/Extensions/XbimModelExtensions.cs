using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Xbim.Common.Geometry;
using Xbim.Common.XbimExtensions;

namespace Xbim.ModelGeometry.Scene.Extensions
{
    public static class XbimModelExtensions
    {
        private static int ReadIndex(BinaryReader br, int maxVertexCount)
        {
            if (maxVertexCount <= 0xFF)
                return br.ReadByte();
            if (maxVertexCount <= 0xFFFF)
                return br.ReadUInt16();
            return (int)br.ReadUInt32(); //this should never go over int32
        }

        public static void Read(this XbimMeshGeometry3D m3D, byte[] mesh, XbimMatrix3D? transform = null)
        {
            var indexBase = m3D.Positions.Count;
            var qrd = new XbimQuaternion();
            
            XbimMatrix3D? matrix3D = null;
            if (transform.HasValue)
            {
                qrd = transform.Value.GetRotationQuaternion();
                matrix3D = transform.Value;
            }
            using (var ms = new MemoryStream(mesh))
            {
                using (var br = new BinaryReader(ms))
                {
                    // ReSharper disable once UnusedVariable
                    var version = br.ReadByte(); //stream format version
                    var numVertices = br.ReadInt32();
                    var numTriangles = br.ReadInt32();

                    var uniqueVertices = new List<XbimPoint3D>(numVertices);
                    var vertices = new List<XbimPoint3D>(numVertices * 4); //approx the size
                    var triangleIndices = new List<int>(numTriangles * 3);
                    var normals = new List<XbimVector3D>(numVertices * 4);
                    for (var i = 0; i < numVertices; i++)
                    {
                        double x = br.ReadSingle();
                        double y = br.ReadSingle();
                        double z = br.ReadSingle();
                        var p = new XbimPoint3D(x, y, z);
                        if (matrix3D.HasValue)
                            p = matrix3D.Value.Transform(p);
                        uniqueVertices.Add(p);
                    }
                    var numFaces = br.ReadInt32();

                    for (var i = 0; i < numFaces; i++)
                    {
                        var numTrianglesInFace = br.ReadInt32();
                        if (numTrianglesInFace == 0) continue;
                        var isPlanar = numTrianglesInFace > 0;
                        numTrianglesInFace = Math.Abs(numTrianglesInFace);
                        if (isPlanar)
                        {
                            var normal = br.ReadPackedNormal().Normal;
                            if (!qrd.IsIdentity())
                            {
                                var baseVal = new XbimVector3D(normal.X, normal.Y, normal.Z);
                                XbimQuaternion.Transform(ref baseVal, ref qrd, out normal);
                            }
                            var uniqueIndices = new Dictionary<int, int>();
                            for (var j = 0; j < numTrianglesInFace; j++)
                            {
                                for (var k = 0; k < 3; k++)
                                {
                                    var idx = ReadIndex(br, numVertices);
                                    int writtenIdx;
                                    if (!uniqueIndices.TryGetValue(idx, out writtenIdx)) //we haven't got it, so add it
                                    {
                                        writtenIdx = vertices.Count;
                                        vertices.Add(uniqueVertices[idx]);
                                        uniqueIndices.Add(idx, writtenIdx);
                                        //add a matching normal
                                        normals.Add(normal);
                                    }
                                    triangleIndices.Add(indexBase + writtenIdx);
                                }
                            }
                        }
                        else
                        {
                            var uniqueIndices = new Dictionary<int, int>();
                            for (var j = 0; j < numTrianglesInFace; j++)
                            {
                                for (var k = 0; k < 3; k++)
                                {
                                    var idx = ReadIndex(br, numVertices);
                                    var normal = br.ReadPackedNormal().Normal;
                                    int writtenIdx;
                                    if (!uniqueIndices.TryGetValue(idx, out writtenIdx)) //we haven't got it, so add it
                                    {
                                        writtenIdx = vertices.Count;
                                        vertices.Add(uniqueVertices[idx]);
                                        uniqueIndices.Add(idx, writtenIdx);

                                        if (!qrd.IsIdentity())
                                        {
                                            var baseVal = new XbimVector3D(normal.X, normal.Y, normal.Z);
                                            XbimQuaternion.Transform(ref baseVal, ref qrd, out normal);
                                        }
                                        normals.Add(normal);
                                    }
                                    triangleIndices.Add(indexBase + writtenIdx);
                                }
                            }
                        }
                    }

                    m3D.Positions = m3D.Positions.Concat(vertices).ToList();
                    m3D.TriangleIndices = m3D.TriangleIndices.Concat(triangleIndices).ToList();
                    m3D.Normals = m3D.Normals.Concat(normals).ToList();
                }
            }
        }

        //public static XbimMeshGeometry3D GetMesh(this XbimModel xbimModel, IEnumerable<IPersistEntity> items, XbimMatrix3D wcsTransform)
        //{
        //    var m = new XbimMeshGeometry3D();

        //    if (xbimModel.GeometrySupportLevel == 1)
        //    {
        //        // this is what happens for version 1 of the engine
        //        //
        //        foreach (var item in items)
        //        {
        //            var fromModel = item.ModelOf as XbimModel;
        //            if (fromModel == null) 
        //                continue;
        //            var geomDataSet = fromModel.GetGeometryData(item.EntityLabel, XbimGeometryType.TriangulatedMesh);
        //            foreach (var geomData in geomDataSet)
        //            {
        //                // todo: add guidance to the TransformBy method so that users can understand how to stop using it (it's marked absolete)
        //                geomData.TransformBy(wcsTransform);
        //                m.Add(geomData, xbimModel.UserDefinedId); 
        //            }
        //        }
        //    }
        //    else
        //    {
        //        // this is what happens for version 2 of the engine
        //        //
        //        foreach (var item in items)
        //        {
        //            var fromModel = item.ModelOf as XbimModel;
        //            if (fromModel == null || !(item is IfcProduct))
        //                continue;
        //            var context = new Xbim3DModelContext(fromModel);

        //            var productShape =
        //                context.ShapeInstancesOf((IfcProduct) item)
        //                    .Where(s => s.RepresentationType != XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded)
        //                    .ToList(); // this also returns shapes of voids
        //            foreach (var shapeInstance in productShape)
        //            {
        //                IXbimShapeGeometryData shapeGeom = context.ShapeGeometry(shapeInstance.ShapeGeometryLabel);
        //                switch ((XbimGeometryType) shapeGeom.Format)
        //                {
        //                    case XbimGeometryType.PolyhedronBinary:
        //                        m.Read(shapeGeom.ShapeData,
        //                            XbimMatrix3D.Multiply(shapeInstance.Transformation, wcsTransform));
        //                        break;
        //                    case XbimGeometryType.Polyhedron:
        //                        m.Read(((XbimShapeGeometry) shapeGeom).ShapeData,
        //                            XbimMatrix3D.Multiply(shapeInstance.Transformation, wcsTransform));
        //                        break;
        //                }
        //            }
        //        }
        //    }
        //    return m;
        //}
    }
}
