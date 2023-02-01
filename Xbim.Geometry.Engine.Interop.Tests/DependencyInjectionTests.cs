using FluentAssertions;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Abstractions.Extensions;
using Xbim.Geometry.Engine.Interop.Extensions;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;
using Xunit;
using Xunit.DependencyInjection;
using Xunit.DependencyInjection.Logging;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    public class DependencyInjectionTests
    {
        private readonly ITestOutputHelperAccessor testOutputHelper;

        public DependencyInjectionTests(ITestOutputHelperAccessor testOutputHelper)
        {
            this.testOutputHelper = testOutputHelper;
        }

        [InlineData(typeof(IXShapeService))]
        [InlineData(typeof(IXGeometryPrimitives))]
        //[InlineData(typeof(IXBRepDocumentManager))]   // Issues with running under another AppDomain.
        [InlineData(typeof(IXGeometryConverterFactory))]
        [InlineData(typeof(IXbimGeometryEngine))]
        [InlineData(typeof(IXbimManagedGeometryEngine))]
        [InlineData(typeof(IXbimGeometryServicesFactory))]
        //
        [Theory]
        public void CanResolveTypes(Type type)
        {
            IServiceProvider provider = BuildServices();

            var resolved = provider.GetRequiredService(type);

            Assert.NotNull(resolved);

        }

        [Fact]
        public void GeometryEngineIsScoped()
        {
            IServiceProvider provider = BuildServices();
            var baseEngine = provider.GetRequiredService<IXbimGeometryEngine>();
            using (var scope = provider.CreateScope())
            {

                var engine = scope.ServiceProvider.GetRequiredService<IXbimGeometryEngine>();
                var engine2 = scope.ServiceProvider.GetRequiredService<IXbimGeometryEngine>();

                engine.Should().BeSameAs(engine2);
                baseEngine.Should().NotBeSameAs(engine);

            }
        }

        [Fact]
        public void ManagedGeometryEngineIsScoped()
        {
            IServiceProvider provider = BuildServices();

            var baseEngine = provider.GetRequiredService<IXbimManagedGeometryEngine>();
            using (var scope = provider.CreateScope())
            {

                var engine = scope.ServiceProvider.GetRequiredService<IXbimManagedGeometryEngine>();
                var engine2 = scope.ServiceProvider.GetRequiredService<IXbimManagedGeometryEngine>();

                engine.Should().BeSameAs(engine2);
                baseEngine.Should().NotBeSameAs(engine);
            }
        }

        [Fact]
        public void ManagedGeometryEngineIsAssignable()
        {
            IServiceProvider provider = BuildServices();

            var engine = provider.GetRequiredService<IXbimGeometryEngine>();
            var engine2 = provider.GetRequiredService<IXbimManagedGeometryEngine>();
            engine.Should().BeSameAs(engine2);
            
        }

        [Fact]
        public void CanCreateEngineWithoutDI()
        {

            var model = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3());
            var loggerFactory = Xbim.Common.XbimLogging.LoggerFactory;
            var engine = new XbimGeometryEngine(model, loggerFactory);

            Assert.NotNull(engine);
        }

        [Fact]
        public void CanCreateEngineWithConfigWithoutDI()
        {
            IServiceProvider provider = BuildServices();

            var factory = provider.GetRequiredService<IXbimGeometryServicesFactory>();

            var model = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3());
            var loggerFactory = Xbim.Common.XbimLogging.LoggerFactory;
            var engine = new XbimGeometryEngine(factory, loggerFactory, new Configuration.GeometryEngineOptions { GeometryEngineVersion=XGeometryEngineVersion.V6});

            Assert.NotNull(engine);
        }

        [Fact]
        public void CanLoad3DContextWithoutDI()
        {

            var model = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3());
            var loggerFactory = Xbim.Common.XbimLogging.LoggerFactory;
            var context = new Xbim3DModelContext(model, loggerFactory, XGeometryEngineVersion.V6);

            Assert.NotNull(context);
        }

        [Fact]
        public void CanRegisterAndUnregister()
        {
            IServiceProvider provider = BuildServices();
            var engine = provider.GetRequiredService<IXbimManagedGeometryEngine>();

            
            
            var model = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3());

            model.Tag.Should().BeNull();

            // Act 
            engine.RegisterModel(model);
            model.Tag.Should().BeOfType(typeof(Dictionary<string, object>));
            model.GetTagValue("ModelGeometryService", out IXModelGeometryService service).Should().BeTrue();
            service.Should().NotBeNull();

            engine.UnregisterModel(model);
            model.GetTagValue("ModelGeometryService", out service).Should().BeFalse();
            
        }


        private IServiceProvider BuildServices()
        {
            var services = new ServiceCollection();

            services.AddLogging(opt => opt.AddProvider(new XunitTestOutputLoggerProvider(testOutputHelper)));
            services.AddGeometryServices(opt => opt.Configure(o => o.GeometryEngineVersion = XGeometryEngineVersion.V5));

            var provider = services.BuildServiceProvider();
            return provider;
        }
    }
}
