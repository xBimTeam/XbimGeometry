    using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
    public interface IXbimTriangulatedModelBuilder
    {
        /// <summary>
        /// Called to initialise the build
        /// </summary>
        void BeginBuild();
        /// <summary>
        /// Called after BeginBuild
        /// </summary>
        /// <param name="numPoints">The number of unique vertices in the model</param>
        void BeginVertices(uint numPoints);
        /// <summary>
        /// Called after BeginVertices, once for each unique vertex
        /// </summary>
        /// <param name="point3D"></param>
        void AddVertex(XbimPoint3D point3D);
        /// <summary>
        /// Called when all unique vertices have been added 
        /// </summary>
        void EndVertices();
        /// <summary>
        /// Called after EndVertices
        /// </summary>
        /// <param name="numFaces">Number of faces in the model</param>
        void BeginFaces(ushort numFaces);
        /// <summary>
        /// Called after BeginFaces, onces for each face
        /// </summary>
        void BeginFace();
        /// <summary>
        /// Called after BeginFace
        /// </summary>
        /// <param name="numNormals">number of normals describing the face, either 1 if the face is planar or 1 normal for each vertices that defines the face</param>
        void BeginNormals(ushort numNormals);
        /// <summary>
        /// Called after BegingNormals, once for each normal to add to face
        /// </summary>
        /// <param name="normal"></param>
        void AddNormal(XbimVector3D normal);
        /// <summary>
        /// Called after last normal has been added
        /// </summary>
        void EndNormals();
        /// <summary>
        /// Called after EndNormal
        /// </summary>
        /// <param name="numPolygons">Number of polygons which make the face</param>
        void BeginPolygons(ushort numPolygons);
        /// <summary>
        /// Called after BeginPolygons once for each Polygon
        /// </summary>
        void BeginPolygon();
        /// <summary>
        /// Called after BeginPolygon, once for each triangulated area that describes the polygon
        /// </summary>
        /// <param name="meshType">The type of triangulation, mesh, fan, triangles etc</param>
        /// <param name="indicesCount"></param>
        void BeginTriangulation(TriangleType meshType, uint indicesCount);
        /// <summary>
        /// Called after BeginTriangulation, once for each index, with respect to the triangulation type
        /// </summary>
        /// <param name="index">index into the list of unique vertices</param>
        void AddTriangleIndex(uint index);
        /// <summary>
        /// Triangulation complete
        /// </summary>
        void EndTriangulation();
        /// <summary>
        /// Polygon definition complete
        /// </summary>
        void EndPolygon();
        /// <summary>
        /// All polygon definitions complete
        /// </summary>
        void EndPolygons();
        /// <summary>
        /// Face complete
        /// </summary>
        void EndFace();
        /// <summary>
        /// All faces complete
        /// </summary>
        void EndFaces();
        /// <summary>
        /// Called before a child is transmitted, calling sequence is same as parent
        /// </summary>
        void BeginChild();
        /// <summary>
        /// Called when child is completed
        /// </summary>
        void EndChild();
        /// <summary>
        /// Model build complete
        /// </summary>
        void EndBuild();
    }
}
