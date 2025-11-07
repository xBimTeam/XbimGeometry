using System;
using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXBRepDocument : IDisposable
    {
        /// <summary>
        /// Create a top level assembly and initialises its shape to an empty IXCompound
        /// </summary>
        /// <param name="name">name of assembly</param>
        /// <returns></returns>
        IXBRepDocumentItem CreateAssembly(string name);
        /// <summary>
        /// Creates a top level assembly and initialises its shape to assemblyShape
        /// </summary>
        /// <param name="name">name of assembly</param>
        /// <param name="assemblyShape">initial shape state</param>
        /// <returns></returns>
        IXBRepDocumentItem CreateAssembly(string name, IXShape assemblyShape);
        bool RemoveAssembly(IXBRepDocumentItem item);
        IXVisualMaterial AddVisualMaterial(IXVisualMaterial visMaterial);
        void SetMaterial(IXShape shape, IXVisualMaterial visMaterial);
        IXVisualMaterial GetMaterial(IXBRepDocumentItem item);
        IXVisualMaterial GetMaterial(IXShape shape);
        IEnumerable<IXVisualMaterial> GetVisualMaterials();
        IEnumerable<IXBRepDocumentItem> GetMaterials();
        /// <summary>
        /// Adds a shape to the document root, this shape may be shared or referred to by other shapes in the document
        /// </summary>
        /// <param name="id">Typically the entity label of the Ifc representation</param>
        /// <param name="shape"></param>
        /// <returns></returns>
        IXBRepDocumentItem AddShape(int id, IXShape shape);
        /// <summary>
        /// Gets a shape by its id stored in the root of the document
        /// </summary>
        /// <param name="id"></param>
        /// <returns>Shape or Null if the shape does not exists</returns>
        IXShape GetShape(int id);

        //IEnumerable<IXStorageItem> FreeShapes { get; }
        IEnumerable<IXBRepDocumentItem> Shapes { get; }
        IXBRepDocumentItem RootItem { get; }

        void UpdateAssemblies();


        double? ConversionFactorForOneMeter { get; set; }
        double? PrecisionFactor { get; set; }
    }
}
