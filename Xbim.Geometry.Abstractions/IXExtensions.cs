using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    public static class IXExtensions
    {
        public static IXPoint EdgeStartPoint(this IXEdge edge)
        {
            return edge?.EdgeStart?.VertexGeometry;
        }
        public static IXPoint EdgeEndPoint(this IXEdge edge)
        {
            return edge?.EdgeEnd?.VertexGeometry;
        }
    }
}
