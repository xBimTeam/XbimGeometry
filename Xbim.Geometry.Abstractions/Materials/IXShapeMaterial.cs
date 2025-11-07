using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeMaterial : IXShapeMaterialItem
    {
        /// <summary>
        /// Optional dictionary of Colours, the int key is the id of the IfcGeometricRepresentationContext that the colour is to be used in
        /// </summary>
        IDictionary<int, IXShapeColour> MaterialColours { get; }
        /// <summary>
        /// Identifier for the material, it can be made unique bu the Category
        /// </summary>
        string Name { get; set; }
        /// <summary>
        /// Optional description
        /// </summary>
        string Description { get; set; }
        /// <summary>
        /// Sub-Qualifier of the name for distinguishing uniqueness
        /// </summary>
        string Category { get; set; }


    }
}
