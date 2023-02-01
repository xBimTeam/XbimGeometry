using Microsoft.Extensions.Logging;
using Xbim.Common;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Interop
{
    /// <summary>
    /// A factory providing access to the native Geometry Engine and related services
    /// </summary>
    public class XbimGeometryServicesFactory : IXbimGeometryServicesFactory
    {
        //static XbimGeometryServicesFactory()
        //{

        //    //// We need to wire in a custom assembly resolver since Xbim.Geometry.Engine is 
        //    //// not located using standard probing rules (due to way we deploy processor specific binaries)
        //    //AppDomain currentDomain = AppDomain.CurrentDomain;
        //    //currentDomain.AssemblyResolve += XbimCustomAssemblyResolver.ResolverHandler;


        //}

        /// <inheritdoc/>
        public IXGeometryConverterFactory GeometryConverterFactory { get; }

        /// <inheritdoc/>
        public XbimGeometryServicesFactory(IXGeometryConverterFactory xGeometryConverterFactory)
        {
            GeometryConverterFactory = xGeometryConverterFactory;
        }

        /// <inheritdoc/>
        public IXModelGeometryService CreateModelGeometryService(IModel model, ILoggerFactory loggerFactory)
        {
            return GeometryConverterFactory.CreateModelGeometryService(model, loggerFactory);
        }

        /// <inheritdoc/>
        public IXbimGeometryEngine CreateGeometryEngineV5(IModel model, ILoggerFactory loggerFactory)
        {
            return GeometryConverterFactory.CreateGeometryEngineV5(model, loggerFactory);
        }

        /// <inheritdoc/>
        public IXGeometryEngineV6 CreateGeometryEngineV6(IModel model, ILoggerFactory loggerFactory)
        {
            return GeometryConverterFactory.CreateGeometryEngineV6(model, loggerFactory);
        }

        /// <inheritdoc/>
        public IXbimGeometryEngine CreateGeometryEngine(XGeometryEngineVersion version, IModel model, ILoggerFactory loggerFactory)
        {
            return GeometryConverterFactory.CreateGeometryEngine(version, model, loggerFactory);
        }

    }
}
