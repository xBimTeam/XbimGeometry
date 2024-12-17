using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Xbim.Common.Configuration;
using Xbim.Geometry.Abstractions;
using Xunit.DependencyInjection;
using Xunit.DependencyInjection.Logging;
namespace Xbim.Geometry.Engine.Tests
{
    public class Startup
    {
#if DEBUG

        const LogLevel DefaultLogLevel = LogLevel.Debug;
#else
        const LogLevel DefaultLogLevel = LogLevel.Information;
#endif
        public void Configure(ILoggerFactory loggerFactory, IServiceProvider serviceProvider)
        {

          // Nothing to do

        }

        public void ConfigureServices(IServiceCollection services)
        {

            services
                .AddLogging(configure => configure
                    .SetMinimumLevel(DefaultLogLevel)
                    .AddXunitOutput()
                    .AddConsole())
                .AddXbimToolkit(configure => configure
                    .AddMemoryModel()
                    .AddGeometryServices(builder => builder.Configure(c => c.GeometryEngineVersion = XGeometryEngineVersion.V5))
                    )
                ;


            // Re-use this Service Collection in the internal xbim DI
            // We can't substitute Xunit.DependencyInjection's IServiceProvider directly since it's a scoped provider which has a per test lifetime
            // and we need the root ServiceProvider. This means we have two ServiceProvider instances in the tests.

            XbimServices.Current.UseExternalServiceCollection(services);



        }
    }

}
