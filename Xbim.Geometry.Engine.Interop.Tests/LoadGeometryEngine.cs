using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting.Logging;
using System;
using System.IO;
using System.Reflection;
using Xbim.Ifc4.GeometryResource;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class LoadGeometryEngine
    {
        [TestMethod]
        public void SimpleLoad()
        {
            var mm = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3());
            var geometryEngineV5 = XbimGeometryEngine.CreateGeometryEngineV5(mm, new NullLoggerFactory());
            geometryEngineV5.Should().NotBeNull();
            var modelGeometryService = XbimGeometryEngine.CreateModelGeometryService(mm, new NullLoggerFactory());
            modelGeometryService.Should().NotBeNull();
        }

       

        [TestMethod]
        public void TestLogging()
        {           
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using var loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
                var ge = XbimGeometryEngine.CreateGeometryEngineV5(m, loggerFactory);
                using (var txn = m.BeginTransaction("new"))
                {
                    var pline = m.Instances.New<IfcPolyline>();
                    ge.CreateCurve(pline);
                    
                }

            }
        }

       
    }
}
