using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using System;
using System.Linq;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{

    public class PrimitiveGeometryTests
    {


        private readonly ILogger _logger;
        private readonly ILoggerFactory _loggerFactory;
        private readonly IXbimGeometryServicesFactory factory;

        public PrimitiveGeometryTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            this.factory = factory;
            this._loggerFactory = loggerFactory;
            _logger = loggerFactory.CreateLogger<PrimitiveGeometryTests>();
        }


        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void can_build_ifc_faceted_brep(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Primitives\ifc_faceted_brep.ifc"))
            {
                var shape = model.Instances.OfType<IIfcFacetedBrep>().FirstOrDefault();
                shape.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, _logger);
                geom.Count.Should().Be(1);
                geom.First().Volume.Should().BeApproximately(3232.386, 5e-3);
            }
        }

        /// <summary>
        /// This test has a blade shape solid which is defined to give a negative volume, but is corrected in V6 now and back ported to V5
        /// </summary>
        /// <param name="engineVersion"></param>
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void can_build_closed_shell(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Primitives\faulty_closed_shell.ifc"))
            {
                var shape = model.Instances.OfType<IIfcClosedShell>().FirstOrDefault();
                shape.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, _logger).FirstOrDefault();
                if (engineVersion == XGeometryEngineVersion.V5)
                    geom.Volume.Should().BeApproximately(-136033.82966702414, 1e-5);
                else //fixed in V6
                    geom.Volume.Should().BeApproximately(136033.82966702414, 1e-5);
            }
        }
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void can_build_poorly_aligned_planar_faces(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Primitives\poor_face_planar_fidelity.ifc"))
            {
                var shape = model.Instances.OfType<IIfcClosedShell>().FirstOrDefault();
                shape.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, _logger).FirstOrDefault();
                geom.Should().NotBeNull();
            }
            if (engineVersion == XGeometryEngineVersion.V6) Console.WriteLine("V6 produces a different result from V5, investigation is required");
        }


    }
}
