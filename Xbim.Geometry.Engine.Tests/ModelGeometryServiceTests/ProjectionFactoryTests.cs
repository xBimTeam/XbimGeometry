using FluentAssertions;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using System;
using System.Linq;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;
using Xbim.Ifc2x3.PresentationDimensioningResource;

namespace Xbim.Geometry.Engine.Tests
{

    public class ProjectionFactoryTests
    { 

        private readonly ILoggerFactory _loggerFactory;
        private readonly IXbimGeometryServicesFactory factory;
        readonly IXModelGeometryService _modelSvc;


        public ProjectionFactoryTests(ILoggerFactory loggerFactory, IXbimGeometryServicesFactory factory)
        {
            _loggerFactory = loggerFactory;
            this.factory = factory;
            var dummyModel = new MemoryModel(new EntityFactoryIfc4());
            _modelSvc = factory.CreateModelGeometryService(dummyModel, _loggerFactory);
        }


        [Theory()]
        [InlineData("TestFiles/Brep/Flow Terminal.brep")]
        public void Can_create_footprint_with_exact_alg(string shapePath)
        {
            var projectionFactory = _modelSvc.ProjectionFactory;
            var shapeFactory = _modelSvc.ShapeFactory;
            string brep = File.ReadAllText(shapePath);
            var shape = shapeFactory.Convert(brep);

            shape.Should().NotBeNull();

            var footprint = projectionFactory.CreateFootprint(shape);

            footprint.Should().NotBeNull();
        }

        [Theory()]
        [InlineData("TestFiles/Brep/Flow Terminal.brep")]
        public void Can_create_footprint_with_polyhedral_simplification_alg(string shapePath)
        {
            var projectionFactory = _modelSvc.ProjectionFactory;
            var shapeFactory = _modelSvc.ShapeFactory;
            string brep = File.ReadAllText(shapePath);
            var shape = shapeFactory.Convert(brep);

            shape.Should().NotBeNull();

            var footprint = projectionFactory.CreateFootprint(shape, false);

            footprint.Should().NotBeNull();
        }
    }

}

