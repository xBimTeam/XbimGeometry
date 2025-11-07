using System.Collections.Generic;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXMaterialsIndex
    {
        IEnumerable<IXShapeColour> Colours { get; }
        IEnumerable<IXShapeMaterial> Materials { get; }
        IXShapeColour GetShapeColour(IIfcGeometricRepresentationItem geomRepItem);
        IIfcMaterial GetMaterial(IIfcObjectDefinition objectDef);
        IXShapeColour GetMaterial(IIfcGeometricRepresentationItem geomRepItem);
        IIfcMaterial GetMaterial(IIfcPropertyDefinition propertyDef);
    }
}
