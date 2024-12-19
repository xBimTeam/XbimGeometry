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
using System.Reflection.PortableExecutable;
using NetTopologySuite.Geometries;

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
        [InlineData("TestFiles/Brep/IfcClippingResult.brep")]
        public void CanCreateValidRefctangularFootprintForAnIfcBeamGeometry(string shapePath)
        {
            var projectionFactory = _modelSvc.ProjectionFactory;
            var shapeFactory = _modelSvc.ShapeFactory;
            string brep = File.ReadAllText(shapePath);
            var shape = shapeFactory.Convert(brep);

            // Act
            var footprint = projectionFactory.CreateFootprint(shape);
            using MemoryStream ms = new MemoryStream();
            using (var writer = new BinaryWriter(ms))
            {
                footprint.Write(writer);
                writer.Close();
            }
            var reader = new NetTopologySuite.IO.WKBReader();
            var polygon = reader.Read(ms.ToArray()) as Polygon;

            // Assert 
            polygon.Should().NotBeNull("Geometry is a Polygon.");
            polygon.IsValid.Should().BeTrue("Polygon is valid.");
            polygon.IsSimple.Should().BeTrue("Polygon is simple.");

            var envelope = polygon.Envelope;
            var polygonNorm = polygon.Normalized();
            var envelopeNorm = envelope.Normalized();

            polygonNorm.EqualsExact(envelopeNorm).Should().BeTrue("Polygon is a perfect rectangle aligned to the axes.");
            polygon.Coordinates.Length.Should().Be(5, "Polygon has the correct number of coordinates for a rectangle.");
        }


        [Theory()]
        [InlineData("TestFiles/Brep/Flow Terminal.brep")]
        public void CanCreateFootprintWithExactAlgorithm(string shapePath)
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
        public void CanCreateFootprintWithPolyhedralSimplificationAlgorithm(string shapePath)
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

