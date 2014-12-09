using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media;
using System.Windows.Media.Media3D;
using Xbim.XbimExtensions;

namespace Xbim.ModelGeometry.Scene
{
    public class WpfMeshGeometry3D : IXbimMeshGeometry3D
    {
        public GeometryModel3D WpfMesh = new GeometryModel3D();
        XbimMeshFragmentCollection meshes = new XbimMeshFragmentCollection();
        
        public static implicit operator GeometryModel3D(WpfMeshGeometry3D mesh)
        {
            return mesh.WpfMesh;
        }

        private MeshGeometry3D Mesh
        {
            get
            { 
                return WpfMesh.Geometry as MeshGeometry3D;
            }
        }
        public IList<Point3D> Positions
        {
            get { return Mesh.Positions; }
        }

        public IList<Vector3D> Normals
        {
            get { return Mesh.Normals; }
        }

        public IList<int> TriangleIndices
        {
            get { return Mesh.TriangleIndices; }
        }



        public XbimMeshFragmentCollection Meshes
        {
            get { return meshes; }
            set
            {
                meshes = new XbimMeshFragmentCollection(value);
            }
        }

        /// <summary>
        /// Do not use this rather create a XbimMeshGeometry3D first and construc this from it, appending WPF collections is slow
        /// </summary>
        /// <param name="geometryMeshData"></param>
        public void Append(XbimGeometryData geometryMeshData)
        {
            throw new NotImplementedException();
        }

        IList<Point3D> IXbimMeshGeometry3D.Positions
        {
            get { return Mesh.Positions; }
            set
            {
                Mesh.Positions = new Point3DCollection(value);
            }
        }

        IList<Vector3D> IXbimMeshGeometry3D.Normals
        {
            get { return Mesh.Normals; }
            set
            {
                Mesh.Normals = new Vector3DCollection(value);
            }
        }

        IList<int> IXbimMeshGeometry3D.TriangleIndices
        {
            get { return Mesh.TriangleIndices; }
            set
            {
                Mesh.TriangleIndices = new Int32Collection(value);
            }
        }

        public void MoveTo(IXbimMeshGeometry3D toMesh)
        {
            if (meshes.Any()) //if no meshes nothing to move
            {
                toMesh.BeginUpdate();
                
                toMesh.Positions = this.Positions; 
                toMesh.Normals = this.Normals; 
                toMesh.TriangleIndices = this.TriangleIndices; 
                toMesh.Meshes = this.Meshes; this.meshes.Clear();
                WpfMesh.Geometry = new MeshGeometry3D();
                toMesh.EndUpdate();
            }
        }

        public void BeginUpdate()
        {
            WpfMesh.Geometry = new MeshGeometry3D();
        }

        public void EndUpdate()
        {
            WpfMesh.Geometry.Freeze();
        }




        public GeometryModel3D ToGeometryModel3D()
        {
            return WpfMesh;
        }
    }
}
