using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.GeometryResource;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{

    public class LoadGeometryEngine
    {
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;

        public LoadGeometryEngine(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            this._factory = factory;
            _loggerFactory = loggerFactory;
        }

        [Fact]
        public void SimpleLoad()
        {
            var mm = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3());
            var geometryEngineV5 = _factory.CreateGeometryEngineV5(mm, new NullLoggerFactory());
            geometryEngineV5.Should().NotBeNull();
            var modelGeometryService = _factory.CreateModelGeometryService(mm, new NullLoggerFactory());
            modelGeometryService.Should().NotBeNull();
        }

       

        [Fact]
        public void TestLogging()
        {           
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                
                var ge = _factory.CreateGeometryEngineV5(m, _loggerFactory);
                using (var txn = m.BeginTransaction("new"))
                {
                    var pline = m.Instances.New<IfcPolyline>();
                    ge.CreateCurve(pline);
                    
                }

            }
        }

       
    }
}
