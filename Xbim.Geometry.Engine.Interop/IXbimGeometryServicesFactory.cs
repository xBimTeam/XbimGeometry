using Microsoft.Extensions.Logging;
using Xbim.Common;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Interop
{
    /// <summary>
    /// Interfaces defining a factory used to construct native a Geometry engine and associated resources
    /// </summary>
    public interface IXbimGeometryServicesFactory
    {

        /// <summary>
        /// Gets the native <see cref="IXGeometryConverterFactory"/>
        /// </summary>
        IXGeometryConverterFactory GeometryConverterFactory { get; }

        /// <summary>
        /// Creates a new native Geometry Engine for the provided version
        /// </summary>
        /// <param name="version"></param>
        /// <param name="model"></param>
        /// <param name="loggerFactory"></param>
        /// <returns>A native <see cref="IXbimGeometryEngine"/> implementation</returns>
        IXbimGeometryEngine CreateGeometryEngine(XGeometryEngineVersion version, IModel model, ILoggerFactory loggerFactory);

        /// <summary>
        /// Creates a new native v5 Geometry Engine
        /// </summary>
        /// <param name="model"></param>
        /// <param name="loggerFactory"></param>
        /// <returns>A native <see cref="IXbimGeometryEngine"/> implementation</returns>
        IXbimGeometryEngine CreateGeometryEngineV5(IModel model, ILoggerFactory loggerFactory);
        /// <summary>
        /// Creates a new native v6 Geometry Engine
        /// </summary>
        /// <param name="model"></param>
        /// <param name="loggerFactory"></param>
        /// <returns></returns>
        IXGeometryEngineV6 CreateGeometryEngineV6(IModel model, ILoggerFactory loggerFactory);
        /// <summary>
        /// Creates a low level <see cref="IXModelGeometryService"/> providing low level native access to Geometry services
        /// </summary>
        /// <param name="model"></param>
        /// <param name="loggerFactory"></param>
        /// <returns>A native <see cref="IXbimGeometryEngine"/> implementation</returns>
        IXModelGeometryService CreateModelGeometryService(IModel model, ILoggerFactory loggerFactory);
    }
}