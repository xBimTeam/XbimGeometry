using System.Collections.Generic;
using System.IO;
using Xbim.Common.Geometry;
using Xbim.Common.XbimExtensions;

// Does this code need to be moved to essentials and reused from there?
namespace Xbim.Temp.Scene
{
    class XbimMesher
    {
        internal List<XbimPoint3D> Positions = new List<XbimPoint3D>();
        internal List<XbimVector3D> Normals = new List<XbimVector3D>();
        internal List<int> Indices = new List<int>();

        internal List<double> PositionsAsDoubleList(double divider)
        {
            List<double> ret = new List<double>(Positions.Count * 3);
            foreach (var pos in Positions)
            {
                ret.Add(pos.X / divider);
                ret.Add(pos.Y / divider);
                ret.Add(pos.Z / divider);
            }
            return ret;
        }

        internal List<double> NormalsAsDoubleList()
        {
            List<double> ret = new List<double>();
            foreach (var nrm in Normals)
            {
                ret.Add(nrm.X);
                ret.Add(nrm.Y);
                ret.Add(nrm.Z);
            }
            return ret;
        }

        internal void AddShape(IGeometryStoreReader geomReader, XbimShapeInstance shapeInstance)
        {
            // XbimMatrix3D modelTransform = XbimMatrix3D.Identity;
            IXbimShapeGeometryData shapeGeom = geomReader.ShapeGeometry(shapeInstance.ShapeGeometryLabel);
            if (shapeGeom.Format != (byte)XbimGeometryType.PolyhedronBinary)
                return;
            // var transform = XbimMatrix3D.Multiply(, modelTransform);
            AddMesh(shapeGeom.ShapeData, shapeInstance.Transformation);
        }
        
        internal void AddMesh(byte[] mesh, XbimMatrix3D? transform = null)
        {
            int indexBase = Positions.Count;
            bool needRotate = false;
            bool needTransform = false;
            XbimQuaternion xq = new XbimQuaternion(0.0, 0.0, 0.0, 1.0);
            XbimMatrix3D transformValue = XbimMatrix3D.Identity;

            if (transform.HasValue)
            {
                transformValue = transform.Value;
                    needTransform = !transformValue.IsIdentity;
                xq = transformValue.GetRotationQuaternion();
                // we have to build a rotation transform from the quaternion (to tranform normals later on)
                needRotate = !xq.IsIdentity();
            }
            using (var ms = new MemoryStream(mesh))
            using (var br = new BinaryReader(ms))
            {
                var t = br.ReadShapeTriangulation();
                List<float[]> pts;
                List<int> idx;
                t.ToPointsWithNormalsAndIndices(out pts, out idx);

                // add to lists
                //
                Positions.Capacity += pts.Count;
                Normals.Capacity += pts.Count;
                Indices.Capacity += idx.Count;
                foreach (var floatsArray in pts)
                {
                    var tmpPosition = new XbimPoint3D(floatsArray[0], floatsArray[1], floatsArray[2]);
                    if (needTransform)
                        tmpPosition = transformValue.Transform(tmpPosition);
                    Positions.Add(tmpPosition);

                    var tmpNormal = new XbimVector3D(floatsArray[3], floatsArray[4], floatsArray[5]);
                    if (needRotate) //transform the normal if we have to
                        XbimQuaternion.Transform(ref tmpNormal, ref xq, out tmpNormal);
                    Normals.Add(tmpNormal);
                }
                foreach (var index in idx)
                {
                    Indices.Add(index + indexBase);
                }
            }
        }
    }
}
