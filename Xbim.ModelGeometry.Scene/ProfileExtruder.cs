using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;
using Xbim.Tessellator;

namespace Xbim.ModelGeometry.Scene
{
    class ProfileExtruder
    {

        static XbimVector3D AsVector(XbimPoint3D value)
        {
            return new XbimVector3D(
                value.X,
                value.Y,
                value.Z
            );
        }

        static XbimPoint3D AsPoint(XbimVector3D value)
        {
            return new XbimPoint3D(
                value.X,
                value.Y,
                value.Z
            );
        }

        public static XbimShapeGeometry ExtrudeCircle(IList<XbimPoint3D> positionPath, double radius, int pointsOnCircle, bool moveMinShapeToOrigin = false)
        {
            double deltaAngle = 2 * Math.PI / pointsOnCircle;

            var silhNormals = new List<XbimPoint3D>(pointsOnCircle);
            var silhPositions = new List<XbimPoint3D>(pointsOnCircle);
            for (int i = 0; i < pointsOnCircle; i++)
            {
                var ang = deltaAngle * i;
                silhPositions.Add(new XbimPoint3D(radius * Math.Cos(ang), radius * Math.Sin(ang), 0));
                silhNormals.Add(new XbimPoint3D(Math.Cos(ang), Math.Sin(ang), 0)); // normal to the specified point
            }

            // add points at beginning and end of poistionPath to specificy entry and exit directions
            //
            var directionIn = positionPath[1] - positionPath[0];
            var directionInPoint = positionPath[0] - directionIn;
            var directionOut = positionPath[positionPath.Count - 1] - positionPath[positionPath.Count - 2];
            var directionOutPoint = positionPath[positionPath.Count - 1] + directionOut;
            positionPath.Insert(0, directionInPoint);
            positionPath.Add(directionOutPoint);
            var segmentsCount = positionPath.Count - 3;
            var positionsCount = positionPath.Count - 2;

            // prevDirection is Initialised normal to the circle creation to 0,1,0 vector
            //
            XbimPoint3D prevDirection = new XbimPoint3D(0, 1, 0);
            XbimPoint3D[] projected_nrm = new XbimPoint3D[(silhPositions.Count)];
            XbimPoint3D[] projected_pts = new XbimPoint3D[(silhPositions.Count)];
            for (int i = 0; i < silhPositions.Count; ++i)
            {
                projected_pts[i] = new XbimPoint3D(silhPositions[i].X, 0, silhPositions[i].Y);
                projected_nrm[i] = new XbimPoint3D(silhNormals[i].X, 0, silhNormals[i].Y);
            }

            XbimPoint3D[] positions = new XbimPoint3D[positionsCount * silhPositions.Count];
            XbimPoint3D[] normals = new XbimPoint3D[positionsCount * silhPositions.Count];
            int icnt = 0;
            for (int i = 1; i < positionPath.Count - 1; ++i)
            {
                var thisDirection = new XbimPoint3D(
                    positionPath[i].X - positionPath[i - 1].X,
                    positionPath[i].Y - positionPath[i - 1].Y,
                    positionPath[i].Z - positionPath[i - 1].Z);
                XbimMatrix3D thisRot = XbimMatrix3D.CreateRotation(
                    prevDirection,
                    thisDirection
                    );

                for (int j = 0; j < silhPositions.Count; ++j)
                {
                    projected_nrm[j] = XbimPoint3D.Multiply(projected_nrm[j], thisRot);
                    projected_pts[j] = XbimPoint3D.Multiply(projected_pts[j], thisRot);
     
                    // set position and normal
                    positions[icnt] = projected_pts[j] + (XbimVector3D)positionPath[i];
                    normals[icnt++] = projected_nrm[j];
                }
                prevDirection = thisDirection;
            }
            List<int> indices = new List<int>(segmentsCount * pointsOnCircle * 6); // two triangles for each division for each segment
            for (int iseg = 0; iseg < segmentsCount; ++iseg)
            {
                for (int iquad = 0; iquad < pointsOnCircle; ++iquad)
                {
                    // indices for quad
                    int p0 = (iseg + 0) * pointsOnCircle + iquad;
                    int p1 = (iseg + 0) * pointsOnCircle + (iquad + 1) % pointsOnCircle;
                    int p2 = (iseg + 1) * pointsOnCircle + iquad;
                    int p3 = (iseg + 1) * pointsOnCircle + (iquad + 1) % pointsOnCircle;
                    indices.Add(p1); indices.Add(p0); indices.Add(p2);
                    indices.Add(p1); indices.Add(p2); indices.Add(p3);
                }
            }

            return MeshPolyhedronBinary(positions, normals, indices, moveMinShapeToOrigin);
        }

        private static XbimShapeGeometry MeshPolyhedronBinary(IList<XbimPoint3D> positions, IList<XbimPoint3D> normals, IList<int> indices, bool moveMinShapeToOrigin = false)
        {
            XbimShapeGeometry shapeGeometry = new XbimShapeGeometry();
            shapeGeometry.Format = XbimGeometryType.PolyhedronBinary;

            XbimVector3D displacement = new XbimVector3D(0, 0, 0);
            if (moveMinShapeToOrigin)
            {
                // compute min coords
                //
                double minX = double.PositiveInfinity;
                double minY = double.PositiveInfinity;
                double minZ = double.PositiveInfinity;
                foreach (var v in positions)

                {
                    minX = Math.Min(minX, v.X);
                    minY = Math.Min(minY, v.Y);
                    minZ = Math.Min(minZ, v.Z);
                }
                displacement = new XbimVector3D(minX, minY, minZ);
            }
            shapeGeometry.TempOriginDisplacement = new XbimPoint3D(displacement.X, displacement.Y, displacement.Z);

            using (var ms = new MemoryStream(0x4000))
            using (var binaryWriter = new BinaryWriter(ms))
            {
                // Write out header
                uint verticesCount = (uint)positions.Count;
                uint triangleCount = (uint)(indices.Count / 3);
                uint facesCount = 1;
                var boundingBox = XbimRect3D.Empty;
 
                binaryWriter.Write((byte)1); //stream format version			
                // ReSharper disable once RedundantCast
                binaryWriter.Write((UInt32)verticesCount); //number of vertices
                binaryWriter.Write(triangleCount); //number of triangles

                foreach (var pos in positions)
                {
                    var Corrected = (pos - displacement);
                    boundingBox.Union(Corrected);
                    binaryWriter.Write((float)(Corrected.X));
                    binaryWriter.Write((float)(Corrected.Y));
                    binaryWriter.Write((float)(Corrected.Z));
                }
                shapeGeometry.BoundingBox = boundingBox;
                //now write out the faces
                binaryWriter.Write(facesCount);
                
                int numTrianglesInFace = (int)triangleCount;
                //we need to fix this
                var planar = false; //we have a mesh of faces that all have the same normals at their vertices
                if (!planar)
                    numTrianglesInFace *= -1; //set flag to say multiple normals
                // ReSharper disable once RedundantCast
                binaryWriter.Write((Int32)numTrianglesInFace);
                
                foreach (var index in indices)
                {
                    WriteIndex(binaryWriter, (uint)index, verticesCount);
                    XbimPackedNormal n = new XbimPackedNormal(normals[index].X, normals[index].Y, normals[index].Z);
                    n.Write(binaryWriter);
                }
                binaryWriter.Flush();
                ((IXbimShapeGeometryData)shapeGeometry).ShapeData = ms.ToArray();
            }
            return shapeGeometry;
        }


        private static void WriteIndex(BinaryWriter bw, UInt32 index, UInt32 maxInt)
        {
            if (maxInt <= 0xFF)
                bw.Write((byte)index);
            else if (maxInt <= 0xFFFF)
                bw.Write((UInt16)index);
            else
                bw.Write(index);
        }


        /// <summary>
        /// Extrusion function with known problems with the handling of normals.
        /// Prefer the ExtrudeCircle function instead.
        /// Bugfix can take inspiration from the ExtrudeCircle function.
        /// </summary>
        internal static XbimShapeGeometry Extrude(IList<XbimPoint3D> positionPath, IList<XbimPoint3D> silhouette, bool silhouetteClosed, bool posiitonPathIncludeInAndOutDirections = false)
        {
            if (!posiitonPathIncludeInAndOutDirections)
            {
                // need to add points for in and out direction required by the algorithm
                var directionIn = positionPath[1] - positionPath[0];
                var directionInPoint = positionPath[0] - directionIn;

                var directionOut = positionPath[positionPath.Count-1] - positionPath[positionPath.Count - 2];
                var directionOutPoint = positionPath[positionPath.Count - 1] + directionOut;

                positionPath.Insert(0, directionInPoint);
                positionPath.Add(directionOutPoint);
            }

            int segments = positionPath.Count - 2;

            // todo: verts allocation can be removed by ensuring that the order of items calculated is continuos and adding directly
            // to the mesh msh
            XbimPoint3D[] verts = new XbimPoint3D[(silhouette.Count * segments)];

            var startPoint = new XbimPoint3D(
                    positionPath[1].X - positionPath[0].X,
                    positionPath[1].Y - positionPath[0].Y,
                    positionPath[1].Z - positionPath[0].Z);
            XbimMatrix3D m = XbimMatrix3D.CreateRotation(
                new XbimPoint3D(0, 1, 0),
                startPoint
                );

            // initialize silhouette on the x/z plane
            XbimPoint3D[] projected_sil = new XbimPoint3D[(silhouette.Count)];
            for (int i = 0; i < silhouette.Count; ++i)
            {
                XbimPoint3D thisPoint = new XbimPoint3D(silhouette[i].X, 0, silhouette[i].Y);
                thisPoint = XbimPoint3D.Multiply(thisPoint, m);
                thisPoint = new XbimPoint3D(
                    thisPoint.X + positionPath[0].X,
                    thisPoint.Y + positionPath[0].Y,
                    thisPoint.Z + positionPath[0].Z
                );
                projected_sil[i] = thisPoint;
            }

            // initialize plane normals from 1 to n-1 (end points are excluded)
            XbimVector3D[] plane_normals = new XbimVector3D[(positionPath.Count)];
            for (int i = 1; i < plane_normals.Length - 1; ++i)
            {
                XbimVector3D p0 = positionPath[i - 1] - positionPath[i];
                XbimVector3D p1 = positionPath[i + 1] - positionPath[i];
                p0 = p0.Normalized();
                p1 = p1.Normalized();
                plane_normals[i] = (p1 - p0).Normalized();
            }

            for (int i = 1; i < positionPath.Count - 1; ++i)
            {
                for (int j = 0; j < silhouette.Count; ++j)
                {
                    XbimVector3D V = (positionPath[i] - positionPath[i - 1]).Normalized();
                    XbimVector3D P = AsVector(projected_sil[j]);
                    XbimVector3D orig = AsVector(positionPath[i]);
                    XbimVector3D N = plane_normals[i];
                    double d = N.DotProduct(orig); //   dot(N, orig);
                    double t = V.DotProduct(N) != 0.0
                        ? (d - P.DotProduct(N)) / V.DotProduct(N)
                        : 0 /*error*/;
                    // project current projected_sil on next plane along p0.p1 vector
                    verts[j + silhouette.Count * (i - 1)] = projected_sil[j] = AsPoint(P + V * t);
                }
            }

            int prof_count = silhouetteClosed ? silhouette.Count : silhouette.Count - 1;
            var indexCount = (6 * prof_count * (segments - 1)); // was 4 * in original logic with quads
            XbimTriangulatedMesh msh = new XbimTriangulatedMesh(indexCount / 3, 0.000001f);
            Dictionary<int, int> duplicatesRemoved = new Dictionary<int, int>();
            int iProg = 0;

            foreach (var vert in verts)
            {
                duplicatesRemoved.Add(iProg++, msh.AddVertex(new Vec3(vert.X, vert.Y, vert.Z)));
            }
            

            for (int iseg = 0; iseg < segments - 1; ++iseg)
            {
                for (int iquad = 0; iquad < prof_count; ++iquad)
                {
                    //// original logic with quads
                    //ib[(iquad * 4 + iseg * 4 * prof_count + 3)] = (iseg + 0) * silhouette.Length + iquad;
                    //ib[(iquad * 4 + iseg * 4 * prof_count + 2)] = (iseg + 0) * silhouette.Length + (iquad + 1) % silhouette.Length;
                    //ib[(iquad * 4 + iseg * 4 * prof_count + 1)] = (iseg + 1) * silhouette.Length + (iquad + 1) % silhouette.Length;
                    //ib[(iquad * 4 + iseg * 4 * prof_count + 0)] = (iseg + 1) * silhouette.Length + iquad;

                    // revised as triangles:
                    int p3 = duplicatesRemoved[(iseg + 0) * silhouette.Count + iquad];
                    int p2 = duplicatesRemoved[(iseg + 0) * silhouette.Count + (iquad + 1) % silhouette.Count];
                    int p1 = duplicatesRemoved[(iseg + 1) * silhouette.Count + (iquad + 1) % silhouette.Count];
                    int p0 = duplicatesRemoved[(iseg + 1) * silhouette.Count + iquad];
                    msh.AddTriangle(p0, p1, p2, iseg);
                    msh.AddTriangle(p0, p2, p3, iseg);
                }
            }
            // msh.
            msh.BalanceNormals(Math.PI/2);
            return XbimTessellator.MeshPolyhedronBinary(msh);

        }

        internal static void Debug(IList<XbimPoint3D> pts)
        {
            FileInfo f = new FileInfo(@"C:\Users\sgmk2\Desktop\profile.scr");
            using (var fw = f.CreateText())
            {
                fw.WriteLine("3DPOLY");
                foreach (var pt in pts)
                {
                    fw.WriteLine($"{pt.X},{pt.Y},{pt.Z}");
                }
                fw.WriteLine();
            }
        }
    }
}
