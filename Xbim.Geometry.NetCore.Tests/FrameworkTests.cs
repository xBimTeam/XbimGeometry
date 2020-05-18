using Extensions.Logging.ListOfString;
using FluentAssertions;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Services;
namespace Xbim.Geometry.NetCore.Tests
{
    [TestClass]
    public class FrameworkTests
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
                .AddSingleton<IXLoggingService, LoggingService>();
            })
        .ConfigureLogging((hostContext, loggingBuilder) =>
        {
            loggingBuilder.AddProvider(new StringListLoggerProvider(new StringListLogger(new List<string>(), name: "LoggingService")));
        });


        [ClassCleanup]
        static public async System.Threading.Tasks.Task CleanupAsync()
        {
            await serviceHost.StopAsync();
            StringListLogger.Instance.LoggedLines.Clear();
        }

        [TestMethod]
        public void Can_create_native_logging_service()
        {

            var loggingService = serviceHost.Services.GetRequiredService<IXLoggingService>();
            loggingService.LogInformation("Framework Test");
            StringListLogger.Instance.LoggedLines.Should().Contain(x => x.Contains("Native Logger handle obtained"));
            StringListLogger.Instance.LoggedLines.Should().Contain(x => x.Contains("Native Logger attached"));
            StringListLogger.Instance.LoggedLines.Should().Contain(x => x.Contains("Framework Test"));
        }

        [TestMethod]
        public void Can_create_logging_scopes()
        {

            var loggingService = serviceHost.Services.GetRequiredService<IXLoggingService>();
            using (loggingService.Logger.BeginScope("Framework Test Scope"))
            {
                loggingService.LogInformation("Framework Test");
                StringListLogger.Instance.LoggedLines.Should().Contain(x => x.Contains("Framework Test Scope"));
            }
            loggingService.LogInformation("Framework Test out of scope");
            StringListLogger.Instance.LoggedLines.Should().Contain(s=>s.StartsWith("[Information] : Xbim.Geometry.Services.LoggingService[0]\r\n             Framework Test out of scope\r\n"));
        }
        #endregion
    }
}
