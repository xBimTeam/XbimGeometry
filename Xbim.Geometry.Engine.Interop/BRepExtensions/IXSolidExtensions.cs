using System.Collections.Generic;
using Xbim.Geometry.Abstractions;
using System.Linq;

namespace Xbim.Geometry.Engine.Interop.BRepExtensions
{
    public static class IXSolidExtensions
    {
        public static ISet<IXFace> UniqueFaces(this IXSolid solid)
        {
            var vertices = new HashSet<IXFace>(new ShapeEqualityComparer());
            foreach (var face in from shell in solid.Shells
                                 from face in shell.Faces
                                 select face)
            {
                vertices.Add(face);
            }

            return vertices;
        }
        public static ISet<IXVertex> UniqueVertices(this IXSolid solid)
        {
            var vertices = new HashSet<IXVertex>(new ShapeEqualityComparer());
            foreach (var face in from shell in solid.Shells
                                 from face in shell.Faces
                                 select face)
            {
                foreach (var edge in face.OuterBound.EdgeLoop)
                {
                    vertices.Add(edge.EdgeStart);
                    vertices.Add(edge.EdgeEnd);
                }

                foreach (var edge in from wire in face.InnerBounds
                                     from edge in wire.EdgeLoop
                                     select edge)
                {
                    vertices.Add(edge.EdgeStart);
                    vertices.Add(edge.EdgeEnd);
                }
            }

            return vertices;
        }
    }
}
