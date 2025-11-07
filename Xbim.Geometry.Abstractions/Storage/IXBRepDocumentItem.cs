using Microsoft.Extensions.Logging;
using System.Collections.Generic;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXBRepDocumentItem
    {
        string Key { get; }
        string Name { get; }
        bool IsTopLevel { get; }
        bool IsSubShape { get; }
        bool IsShape { get; }
        bool IsFree { get; }
        bool IsAssembly { get; }
        bool IsComponent { get; }
        bool IsCompound { get; }

        bool IsExternalRef { get; }
        bool IsSimpleShape { get; }
        bool IsReference { get; }
        bool IsNull { get; }
        IXBRepDocumentItem AddShape(string name, IXShape shape, bool expand = false);
        IXBRepDocumentItem AddComponent(string name, int shapeId, IXShape shape, IIfcObjectPlacement objPlacement, ILogger logger);
        IXBRepDocumentItem AddComponent(string name, IXBRepDocumentItem component, IIfcObjectPlacement objPlacement, ILogger logger);
        IXBRepDocumentItem AddComponent(string name, IXBRepDocumentItem component, XbimMatrix3D transform);
        IXBRepDocumentItem AddComponent(string name, int shapeId, IXShape shape);
        IXBRepDocumentItem AddSubShape(string name, IXShape shape, bool isExistingPart);
        int NbComponents { get; }
        //IXVisualMaterial VisualMaterial { get; set; }
        IXShape Shape { get; set; }
        IEnumerable<IXBRepDocumentItem> Components { get; }
        IEnumerable<IXBRepDocumentItem> SubShapeStorageItems { get; }
        IEnumerable<IXShape> SubShapes { get; }
        IXBRepDocumentItem ReferredShape { get; }
        IXBRepDocumentItem AddAssembly(string subAssemblyName);
        IXBRepDocumentItem AddAssembly(string subAssemblyName, IIfcObjectPlacement placement);
        IXBRepDocumentItem AddAssembly(string subAssemblyName, XbimMatrix3D transform);
        bool IsStored { get; }
        double? Volume { get; set; }
        double? Area { get; set; }
        double? HeightMin { get; set; }
        double? HeightMax { get; set; }

        double? ThicknessMax { get; set; }
        void SetPlacement(IIfcObjectPlacement objectPlacement);
        /// <summary>
        /// The Ifc Entity label
        /// </summary>
        int IfcId { get; set; }
        /// <summary>
        /// The Ifc Type Id 
        /// </summary>
        short IfcTypeId { get; set; }
        /// <summary>
        /// The Name property of an IfcRoot object
        /// </summary>
        string IfcName { get; set; }
        string Classification { get; set; }
        bool HasOpenings { get; set; }
        bool HasProjections { get; set; }
        bool IsProduct { get; set; }
        bool IsShapeRepresentation { get; set; }
        bool IsShapeRepresentationMap { get; set; }
        bool IsGeometricRepresentationItem { get; set; }
        string IfcType { get; set; }
        int NbMaterialLayers { get; set; }
        void SetStringAttribute(string name, string value);
        string GetStringAttribute(string name);
        void SetIntAttribute(string name, int value);
        int GetIntAttribute(string name);
        void SetDoubleAttribute(string name, double? value);
        double? GetDoubleAttribute(string name);
        /// <summary>
        /// If this storage item is a material, returns all shapes assigned to this material
        /// </summary>
        IEnumerable<IXBRepDocumentItem> ShapesAssignedToMaterial { get; }
        void AddReference(IXBRepDocumentItem referred, XbimMatrix3D? transform);
        IXVisualMaterial VisualMaterial { get; set; }
    }
}