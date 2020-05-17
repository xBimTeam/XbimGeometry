using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXCompound: IXShape
    {
        public string BRepDefinition();
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
        /// The compound contains one or more solids only and no other top level shapes
        /// </summary>
        bool HasSolids { get; }
        /// <summary>
        /// The compound contains  one or more top level solids
        /// </summary>
        bool HasShells { get; }
        /// <summary>
        /// The compound contains  one or more top level shells (not nested in a solid)
        /// </summary>
        bool HasFaces { get; }
        /// <summary>
        /// The compound contains  one or more top level faces (not nested in a shell)
        /// </summary>
        IEnumerable<IXSolid> Solids { get; }
        /// <summary>
        /// Returns enumeration of all the top level shells
        /// </summary>
        IEnumerable<IXShell> Shells { get; }
        /// <summary>
        /// Returns enumeration of all the top level faces
        /// </summary>
        IEnumerable<IXFace> Faces { get; }



    }
}
