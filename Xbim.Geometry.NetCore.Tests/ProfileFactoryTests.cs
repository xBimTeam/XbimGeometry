using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Services;
namespace Xbim.Geometry.NetCore.Tests
{
    [TestClass]
    class ProfileFactoryTests
    {
        #region Setup


        static IHost serviceHost;

        [ClassInitialize]
        static public async System.Threading.Tasks.Task InitialiseAsync(TestContext context)
        {
            serviceHost = CreateHostBuilder().Build();
            await serviceHost.StartAsync();
        }

        public static IHostBuilder CreateHostBuilder() =>
        Host.CreateDefaultBuilder()
            .ConfigureServices((hostContext, services) =>
            {
                services.AddHostedService<GeometryServicesHost>()
                .AddSingleton<IXLoggingService, LoggingService>()
                .AddSingleton<IXShapeService, ShapeService>()
                .AddScoped<IXSolidService>()
                .AddScoped<IXModelService>();
            })
        .ConfigureLogging((hostContext, loggingBuilder) =>
        {
            loggingBuilder.AddConsole((config) => config.IncludeScopes = true).AddDebug();
        });

        
        [ClassCleanup]
        static public async System.Threading.Tasks.Task CleanupAsync()
        {
            await serviceHost.StopAsync();
        }

        [TestMethod]
        public void Can_create_logging_service()
        {
           var ls = serviceHost.Services.GetRequiredService<IXLoggingService>();
        }
        #endregion
    }
}
