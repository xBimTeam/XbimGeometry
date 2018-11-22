using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    public interface IXbimMeshGeometry3D : IXbimTriangulatesToPositionsIndices, IXbimTriangulatesToPositionsNormalsIndices
    {
        IEnumerable<XbimPoint3D> Positions { get; set; }
        IEnumerable<XbimVector3D> Normals { get; set; }
        IList<Int32> TriangleIndices { get; set; }
        XbimMeshFragmentCollection Meshes { get; set; }
        
        bool Add(XbimGeometryData geometryMeshData, short modelId=0);
        XbimMeshFragment Add(IXbimGeometryModel geometryModel, IIfcProduct product, XbimMatrix3D transform, double? deflection, short modelId = 0);

        void MoveTo(IXbimMeshGeometry3D toMesh);
        void BeginUpdate();
        void EndUpdate();
        void ReportGeometryTo(StringBuilder sb);
        /// <summary>
        /// Returns the part of the mesh described in the fragment
        /// </summary>
        /// <param name="frag"></param>
        /// <returns></returns>
        IXbimMeshGeometry3D GetMeshGeometry3D(XbimMeshFragment frag);
        XbimRect3D GetBounds();

        void Add(string mesh, Type productType, int productLabel, int geometryLabel, XbimMatrix3D? transform = null, short modelId=0);
        void Add(string mesh, short productTypeId, int productLabel, int geometryLabel, XbimMatrix3D? transform = null, short modelId = 0);
    }
}
