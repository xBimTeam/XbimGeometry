using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using System;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop.Configuration;


namespace Xbim.Geometry.Engine.Interop.Extensions
{
    public static class GeometryEngineBuilderExtensions
    {
        /// <summary>
        /// Sets the version of the internal xbim Geometry Engine to use
        /// </summary>
        /// <param name="builder"></param>
        /// <param name="version"></param>
        /// <returns>The <see cref="IGeometryEngineBuilder"/> so that additional calls can be chained.</returns>
        public static IGeometryEngineBuilder SetVersion(this IGeometryEngineBuilder builder, XGeometryEngineVersion version)
        {
            builder.Services.Add(ServiceDescriptor.Singleton((IConfigureOptions<GeometryEngineOptions>)new DefaultGeometryEngineConfigurationOptions(version)));
            return builder;
        }

        /// <summary>
        /// Configure the <paramref name="builder"/> with the <see cref="GeometryEngineOptions"/>.
        /// </summary>
        /// <param name="builder">The <see cref="IGeometryEngineBuilder"/> to be configured with <see cref="GeometryEngineOptions"/></param>
        /// <param name="action">The action used to configure the logger factory</param>
        /// <returns>The <see cref="IGeometryEngineBuilder"/> so that additional calls can be chained.</returns>
        public static IGeometryEngineBuilder Configure(this IGeometryEngineBuilder builder, Action<GeometryEngineOptions> action)
        {
            builder.Services.Configure(action);
            return builder;
        }
    }
}
