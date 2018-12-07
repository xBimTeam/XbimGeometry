using Xbim.Common.Geometry;
using Xbim.Ifc4;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    internal class MeshHelper:IXbimMeshReceiver
    {
        public int PointCount;
        public int FaceCount;
        public int TriangleIndicesCount;
        public int TriangleCount;
        public XbimRect3D BoundingBox = XbimRect3D.Empty;

        public void BeginUpdate()
        {
            PointCount = 0;
            FaceCount = 0;
            TriangleIndicesCount = 0;
            TriangleCount = 0;
        }

        public void EndUpdate()
        {
            TriangleCount = TriangleIndicesCount/3;
        }

        public int AddFace()
        {
            FaceCount++;
            return FaceCount;
            
        }

        public int AddNode(int face, double px, double py, double pz, double nx, double ny, double nz, double u, double v)
        {
            PointCount++;
            BoundingBox.Union(new XbimPoint3D(px, py, pz));
            return PointCount;
        }

        public int AddNode(int face, double px, double py, double pz, double nx, double ny, double nz)
        {
           // Console.WriteLine("{0}, {1}, {2} - {3}, {4}, {5}", px, py, pz, nx,ny,nz);
            PointCount++;
            BoundingBox.Union(new XbimPoint3D(px,py,pz));
            return PointCount;
        }

        public int AddNode(int face, double px, double py, double pz)
        {
            
            PointCount++;
            BoundingBox.Union(new XbimPoint3D(px, py, pz));
            return PointCount;
        }

        public void AddTriangle(int face, int a, int b, int c)
        {
            TriangleIndicesCount += 3;
        }

        public void AddQuad(int face, int a, int b, int c, int d)
        {
            TriangleIndicesCount += 6;
        }

        public SurfaceStyling SurfaceStyling { get; set; }

       
      
        
    }
}
