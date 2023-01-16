using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Engine.Interop.Tests.Extensions;
using Xunit.DependencyInjection;
using Xunit.DependencyInjection.Logging;
namespace Xbim.Geometry.Engine.Interop.Tests
{
    public class Startup
    {
#if DEBUG

        const LogLevel DefaultLogLevel = LogLevel.Debug;
#else
        const LogLevel DefaultLogLevel = LogLevel.Information;
#endif
        public void Configure(ILoggerFactory loggerFactory, ITestOutputHelperAccessor accessor) =>
        loggerFactory.AddProvider(new XunitTestOutputLoggerProvider(accessor, (name, level) => level is >= DefaultLogLevel and < LogLevel.None));
        public void ConfigureServices(IServiceCollection services)
        {
            services.AddLogging(configure => configure.SetMinimumLevel(DefaultLogLevel))
            .AddXbimGeometryServices();
            
        }
    }
}
