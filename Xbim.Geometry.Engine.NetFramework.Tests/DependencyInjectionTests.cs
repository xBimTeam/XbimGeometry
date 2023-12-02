using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Xbim.Common.Configuration;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc4.Interfaces;
using Xunit;

namespace Xbim.Geometry.Engine.NetFramework.Tests
{
    public class DependencyInjectionTests
    {
        [Fact]
        public void CanResolveGeometryEngine()
        {

            var engine = XbimServices.Current.ServiceProvider.GetService<IXbimGeometryEngine>();
            Assert.NotNull(engine);
        }

        [Fact]
        public void CanResolveShapeStore()
        {
            var service = XbimServices.Current.ServiceProvider.GetService<IXShapeService>();
            Assert.NotNull(service);
        }

        [Fact]
        public void CanResolveGeometryPrimitives()
        {
            var service = XbimServices.Current.ServiceProvider.GetService<IXGeometryPrimitives>();
            Assert.NotNull(service);
        }

        [Fact]
        public void CanResolveILoggerFactory()
        {
            var logFactory = XbimServices.Current.ServiceProvider.GetService<ILoggerFactory>();

            Assert.NotNull(logFactory);

            Assert.IsType<LoggerFactory>(logFactory);

            var expectedVersion = new Version(2, 1, 0,0);
            Assert.Equal(expectedVersion, logFactory.GetType().Assembly.GetName().Version);
        }

        
    }
}