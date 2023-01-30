using Microsoft.Extensions.DependencyInjection;
using System;

namespace Xbim.Geometry.Engine.Interop.Internal
{
    /// <summary>
    /// Helper providing access to intenal services required to use Geometry Engine
    /// </summary>
    public class XbimGeometryInternalServices
    {

        static Lazy<IServiceProvider> lazyServiceProvider = new Lazy<IServiceProvider>(() =>
        {
            var services = new ServiceCollection();
            services.AddGeometryServices()
                    .AddLogging()
                    //.AddSingleton(loggerFactory)
                    ;
            return services.BuildServiceProvider();
        });

        /// <summary>
        /// Gets a <see cref="IServiceProvider"/> used for resolving xbim Geometry Engine services 
        /// </summary>
        /// <remarks>Used when an external DI service is not employed</remarks>
        public static IServiceProvider ServiceProvider => lazyServiceProvider.Value;
    }
}
