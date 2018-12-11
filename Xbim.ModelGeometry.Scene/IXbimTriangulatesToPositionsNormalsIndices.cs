using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
    public interface IXbimTriangulatesToPositionsNormalsIndices
    {
        /// <summary>
        /// Called to initialise the build
        /// </summary>
        void BeginBuild();
        /// <summary>
        /// Called after BeginBuild
        /// </summary>
        /// <param name="numPoints">The number of unique vertices in the model</param>
        void BeginPoints(uint numPoints);
        /// <summary>
        /// Called after BeginVertices, once for each unique vertex
        /// </summary>
        /// <param name="point3D"></param>
        void AddPosition(XbimPoint3D point3D);
        void AddNormal(XbimVector3D normal);
        /// <summary>
        /// Called when all unique vertices have been added 
        /// </summary>
        void EndPoints();
        /// <summary>
        /// Called after EndNormal
        /// </summary>
        /// <param name="totalNumberTriangles">The total number of triangles in the face</param>
        /// <param name="numPolygons">Number of polygons which make the face</param>
        void BeginPolygons(uint totalNumberTriangles, uint numPolygons);
        /// <summary>
        /// Called after BeginPolygon, once for each triangulated area that describes the polygon
        /// </summary>
        /// <param name="meshType">The type of triangulation, mesh, fan, triangles etc</param>
        /// <param name="indicesCount"></param>
        void BeginPolygon(TriangleType meshType, uint indicesCount);
        /// <summary>
        /// Called after BeginTriangulation, once for each index, with respect to the triangulation type
        /// </summary>
        /// <param name="index">index into the list of unique vertices</param>
        void AddTriangleIndex(uint index);
        /// <summary>
        /// Triangulation complete
        /// </summary>
        void EndPolygon();
        /// <summary>
        /// All polygon definitions complete
        /// </summary>
        void EndPolygons();
        /// <summary>
        /// Model build complete
        /// </summary>
        void EndBuild();

        int PositionCount { get; }
        int TriangleIndexCount { get; }
    }
}
