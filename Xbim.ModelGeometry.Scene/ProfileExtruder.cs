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

        public static XbimShapeGeometry Extrude(IList<XbimPoint3D> positionPath, IList<XbimPoint3D> silhouette, bool silhouetteClosed, bool posiitonPathIncludeInAndOutDirections = false)
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
                    if (t == 0)
                    {
                        string msg = "error";
                    }
                    // project current projected_sil on next plane along p0.p1 vector
                    verts[j + silhouette.Count * (i - 1)] = projected_sil[j] = AsPoint(P + V * t);
                }
            }

            int prof_count = silhouetteClosed ? silhouette.Count : silhouette.Count - 1;
            var indexCount = (6 * prof_count * (segments - 1)); // was 4 * in original logic with quads
            XbimTriangulatedMesh msh = new XbimTriangulatedMesh(indexCount / 3, 0.000001f);


            Dictionary<int, int> duplicatesRemoved = new Dictionary<int, int>();
            int iProg = 0;

            //// FileInfo f = new FileInfo(@"C:\Users\sgmk2\Desktop\pts.scr");
            //using (var fw = f.CreateText())
            //{
                foreach (var vert in verts)
                {
                    //fw.WriteLine($"Point");
                    //fw.WriteLine($"{vert.X},{vert.Y},{vert.Z}");
                    duplicatesRemoved.Add(iProg++, msh.AddVertex(new Vec3(vert.X, vert.Y, vert.Z)));
                }
            // }
                        
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
