using System;
using System.Collections.Generic;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimEmptyGeometryGroup : List<IXbimGeometryModel>, IXbimGeometryModelGroup, IXbimPolyhedron
    {
        static XbimEmptyGeometryGroup _empty;
        public static XbimEmptyGeometryGroup Empty { get { return _empty; } }
        static XbimEmptyGeometryGroup()
        {
            _empty = new XbimEmptyGeometryGroup();
        }
        private  XbimEmptyGeometryGroup() //not intended to be constructed
        {

        }
        public int RepresentationLabel { get { return -1; } set { } }
        public int SurfaceStyleLabel { get { return -1; } set { } }
        public bool IsMap { get { return false; } }

        public XbimRect3D GetBoundingBox() { return XbimRect3D.Empty; }

        public double Volume { get { return 0; } }

        public XbimMatrix3D Transform { get { return XbimMatrix3D.Identity; } }

        public XbimTriangulatedModelCollection Mesh(double deflection) { return new XbimTriangulatedModelCollection(); }

        public String WriteAsString(XbimModelFactors modelFactors) { return "EMPTY"; }

        public XbimRect3D GetAxisAlignedBoundingBox()
        {
            return XbimRect3D.Empty;
        }

       
        public XbimMeshFragment MeshTo(IXbimMeshGeometry3D mesh3D, IIfcProduct product, XbimMatrix3D transform, double deflection, short modelId)
        {
            return default(XbimMeshFragment);
        }


        public XbimPoint3D CentreOfMass
        {
            get { return default(XbimPoint3D); }
        }


       


        public bool Write(string fileName, XbimModelFactors modelFactors)
        {
            return false;
        }


        public IXbimGeometryModel Cut(IXbimGeometryModel openingGeom, XbimModelFactors modelFactors)
        {
            return this;
        }

        public IXbimGeometryModel Union(IXbimGeometryModel toUnion, XbimModelFactors modelFactors)
        {
            return this;
        }


        public IXbimGeometryModel Intersection(IXbimGeometryModel toIntersect, XbimModelFactors modelFactors)
        {
            return this;
        }


        public IXbimGeometryModel Combine(IXbimGeometryModel toCombine, XbimModelFactors modelFactors)
        {
            return this;
        }


        public IXbimPolyhedron ToPolyhedron(XbimModelFactors modelFactors)
        {
            return this;
        }

        public bool WritePly(string fileName, bool ascii = true)
        {
            return false;
        }




        public int VertexCount
        {
            get { return 0; }
        }

        public XbimPoint3D Vertex(int i)
        {
            return default(XbimPoint3D);
        }
        public IList<Int32> Triangulation(double precision)
        {
            return new List<int>(0);
        }


        public IXbimGeometryModel TransformBy(XbimMatrix3D xbimMatrix3D)
        {
            return this;
        }

        public bool HasCurvedEdges
        {
            get { return false; }
        }


        public int FaceCount
        {
            get { throw new NotImplementedException(); }
        }

        public int MergeCoPlanarFaces(double p)
        {
            return 0;
        }
    }
}
