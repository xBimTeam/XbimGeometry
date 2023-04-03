using Castle.DynamicProxy.Generators;
using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{
    public class IfcExampleFilesTests
    {
        
        private ILoggerFactory _loggerFactory;
        private readonly IXbimGeometryServicesFactory _factory;

        public IfcExampleFilesTests(ILoggerFactory loggerFactory, IXbimGeometryServicesFactory factory)
        {
            this._loggerFactory = loggerFactory;
            _factory = factory;
        }

        [Fact]
        public void Can_Build_Beam_Standard_Case()
        {
            //this file contains a set of IShapeProfile beaams and a set of T shaped profile beams
            using var model = MemoryModel.OpenRead("testfiles/ifcExamples/beam-standard-case.ifc");
            var geomEngineV6 = _factory.CreateGeometryEngineV6(model, _loggerFactory);
            foreach (var geomRep in model.Instances.OfType<IIfcSolidModel>())
            {
               var brep = geomEngineV6.Create(geomRep);
                brep.Should().NotBeNull();
            }
        }
        
    }
}
