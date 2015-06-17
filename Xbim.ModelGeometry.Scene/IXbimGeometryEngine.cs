using System;
using Xbim.Common.Geometry;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.Kernel;
using Xbim.Ifc2x3.RepresentationResource;
using Xbim.IO;
using Xbim.XbimExtensions;
using XbimGeometry.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    public interface IXbimGeometryEngine : IGeometryManager
    {
        /// <summary>
        /// Returns the geometry in mixed mode, this is the faster way of henerating the geometry
        /// </summary>
        /// <param name="product"></param>
        /// <param name="xbimGeometryType"></param>
        /// <returns></returns>
        IXbimGeometryModelGroup GetGeometry3D(IfcProduct product);

        /// <summary>
        /// Returns the geometry formatted to a specific type
        /// </summary>
        /// <param name="product"></param>
        /// <param name="xbimGeometryType"></param>
        /// <returns></returns>
        IXbimGeometryModelGroup GetGeometry3D(IfcProduct product, XbimGeometryType xbimGeometryType);
       
        /// <summary>
        /// Initialises the geometry engine and resets any cached data
        /// </summary>
        /// <param name="model"></param>
        void Init(XbimModel model);

        /// <summary>
        /// Returns the 3D geometry for the representation item
        /// </summary>
        /// <param name="repItem"></param>
        /// <returns></returns>
        IXbimGeometryModelGroup GetGeometry3D(IfcRepresentationItem repItem);


        IXbimGeometryModelGroup GetGeometry3D(IfcRepresentation representation);
        /// <summary>
        /// Returns the geometry in 3D of a given type 
        /// </summary>
        /// <param name="solid"></param>
        /// <param name="xbimGeometryType"></param>
        /// <returns></returns>
        IXbimGeometryModelGroup GetGeometry3D(IfcSolidModel solid, XbimGeometryType xbimGeometryType);
        /// <summary>
        /// Returns a geometry object represented by the string data
        /// </summary>
        /// <param name="data"></param>
        /// <param name="xbimGeometryType"></param>
        /// <returns></returns>
        IXbimGeometryModel GetGeometry3D(String data, XbimGeometryType xbimGeometryType);
       
    }
}
