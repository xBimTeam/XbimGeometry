using Xbim.Ifc4;

namespace GeometryTests
{
    internal class MeshHelper:IXbimMeshReceiver
    {
        public int PointCount;
        public int FaceCount;
        public int TriangleIndicesCount;
        public int TriangleCount;

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
            return PointCount;
        }

        public int AddNode(int face, double px, double py, double pz, double nx, double ny, double nz)
        {
           // Console.WriteLine("{0}, {1}, {2} - {3}, {4}, {5}", px, py, pz, nx,ny,nz);
            PointCount++;
            return PointCount;
        }

        public int AddNode(int face, double px, double py, double pz)
        {
            
            PointCount++;
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

        public IPhongMaterial Material { get; set; }
    }
}
