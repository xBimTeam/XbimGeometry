using Microsoft.Extensions.Logging;
using Xbim.Common;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3.GeometryResource;

namespace Xbim.Geometry.Abstractions
{
    public interface IXGeometryConverterFactory
    {
        /// <summary>
        /// Version 5 Geometry Engine for backward compatibility, now deprecated
        /// </summary>
        IXbimGeometryEngine CreateGeometryEngineV5(IModel model, ILoggerFactory loggerFactory);
        /// <summary>
        /// Version 6 Geometry Engine for comparison with V5, this is for backward compatibility, test and develop purposes only. 
        /// It will be deprecated in facour of CreateModelGeometryService
        /// </summary>
        IXGeometryEngineV6 CreateGeometryEngineV6(IModel model, ILoggerFactory loggerFactory);
        /// <summary>
        /// Creates a Root Service to access Geometry Factories that are scoped to the current Model
        /// </summary>
        IXModelGeometryService CreateModelGeometryService(IModel model, ILoggerFactory loggerFactory);

        IXbimGeometryEngine CreateGeometryEngine(XGeometryEngineVersion version,  IModel model, ILoggerFactory loggerFactory);

        /// <summary>
        /// Gets the underlying modelgeoemtry service used by this engine
        /// </summary>
        /// <param name="geometryEngine"></param>
        /// <returns></returns>
        IXModelGeometryService GetUnderlyingModelGeometryService(IXbimGeometryEngine geometryEngine);
    }
}
