namespace Xbim.Geometry.Abstractions
{
    public interface IXCompound : IXShape
    {

        bool IsCompoundsOnly { get; }
        /// <summary>
        /// The compound contains one or more solids only and no other top level shapes
        /// </summary>
        bool IsSolidsOnly { get; }
        /// <summary>
        /// The compound contains  one or more shells only and no other top level shape types
        /// </summary>
        bool IsShellsOnly { get; }

        /// <summary>
        /// The compound contains  one or more faces only and no other top level shape types
        /// </summary>
        bool IsFacesOnly { get; }

        /// <summary>
        /// The compound contains  one or more wires only and no other top level shape types
        /// </summary>
        bool IsWiresOnly { get; }
        /// <summary>
        /// The compound contains  one or more edges only and no other top level shape types
        /// </summary>
        bool IsEdgesOnly { get; }

        /// <summary>
        /// The compound contains one or more compounds 
        /// </summary>
        bool HasCompounds { get; }
        /// <summary>
        /// The compound contains one or more solids (not nested in a compound)
        /// </summary>
        bool HasSolids { get; }
        /// <summary>
        /// The compound contains  one or more top level shells(not nested in a solid)
        /// </summary>
        bool HasShells { get; }
        /// <summary>
        /// The compound contains  one or more top level faces (not nested in a shell)
        /// </summary>
        bool HasFaces { get; }
        /// <summary>
        /// The compound contains  one or more top level wires (not nested in a shape)
        /// </summary>
        bool HasWires { get; }
        /// <summary>
        /// The compound contains  one or more top level edges (not nested in a shape)
        /// </summary>
        bool HasEdges { get; }
        /// <summary>
        /// The compound contains  one or more top level vertices (not nested in a any shape)
        /// </summary>
        bool HasVertices { get; }
        /// <summary>
        /// Returns enumeration of all the top level compounds
        /// </summary>
        IXCompound[] Compounds { get; }
        /// <summary>
        /// Returns enumeration of all the top level solids
        /// </summary>
        IXSolid[] Solids { get; }
        /// <summary>
        /// Returns enumeration of all the top level shells
        /// </summary>
        IXShell[] Shells { get; }
        /// <summary>
        /// Returns enumeration of all the top level faces
        /// </summary>
        IXFace[] Faces { get; }

        /// <summary>
        /// Returns enumeration of all the top level wires
        /// </summary>
        IXWire[] Wires { get; }

        /// <summary>
        /// Returns enumeration of all the top level edges
        /// </summary>
        IXEdge[] Edges { get; }

        /// <summary>
        /// Returns enumeration of all the top level vertices
        /// </summary>
        IXVertex[] Vertices { get; }

        void Add(IXShape shape);

    }
}
