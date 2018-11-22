using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Xbim.ModelGeometry.Scene
{
    public interface IXbimTriangulatesToSimplePositionsNormalsIndices
    {
        /// <summary>
        /// Called to initialise the build
        /// </summary>
        void BeginBuild(uint PointsCount, uint TrianglesCount);
        void BeginPoints(uint PointsCount);
        void AddPoint(float px, float py, float pz, float nx, float ny, float nz);
        void BeginTriangles(uint totalNumberTriangles);
        void AddTriangleIndex(uint index);
        void EndBuild();
    }
}
