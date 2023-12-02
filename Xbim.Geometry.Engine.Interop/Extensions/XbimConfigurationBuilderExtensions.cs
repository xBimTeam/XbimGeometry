using System;
using Xbim.Geometry.Engine.Interop.Configuration;


namespace Xbim.Common.Configuration
{
    /// <summary>
    /// Extension methods for <see cref="IXbimConfigurationBuilder"/>
    /// </summary>
    public static class XbimConfigurationBuilderExtensions
    {
        /// <summary>
        /// Adds xbim Geometry services, including the native dependencies to the current configuration
        /// </summary>
        /// <param name="builder"></param>
        /// <returns></returns>
        public static IXbimConfigurationBuilder AddGeometryServices(this IXbimConfigurationBuilder builder)
        {
            builder.Services.AddXbimGeometryServices();
           
            return builder;
        }


        /// <summary>
        /// Adds xbim Geometry services, including the native dependencies to the current configuration with the supplied configuration
        /// </summary>
        /// <param name="builder"></param>
        /// <param name="configure">Delegate configuring the xbim Geometry service</param>
        /// <returns></returns>
        public static IXbimConfigurationBuilder AddGeometryServices(this IXbimConfigurationBuilder builder, Action<IGeometryEngineBuilder> configure)
        {
            builder.Services.AddXbimGeometryServices(configure);

            return builder;
        }
    }
}
