using System;
using System.Collections.Generic;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
    public interface IXbimPolyhedron : IXbimGeometryModel
    {
        /// <summary>
        /// Writes the polyhedron to a file in the Stanford PLY format
        /// </summary>
        /// <param name="fileName"></param>
        /// <param name="ascii"></param>
        /// <returns></returns>
        bool WritePly(string fileName, bool ascii=true);

        int VertexCount { get; }
        int FaceCount { get; }
        XbimPoint3D Vertex(int i);
        IList<Int32> Triangulation(double precision);
        int MergeCoPlanarFaces(double p);
    }
}
