using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimTriangulatedModel
    {
        public XbimTriangulatedModel(byte[] triangles, XbimRect3D bounds, int representationLabel, int surfaceStylelabel)
        {
            Triangles = triangles;
            RepresentationLabel = representationLabel;
            SurfaceStyleLabel = surfaceStylelabel;
            Bounds = bounds;
        }

        public byte[] Triangles { get; set; }

        public int RepresentationLabel { get; set; }

        public int SurfaceStyleLabel { get; set; }

        public XbimRect3D Bounds;
    }
}
