using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using System;
using System.Linq;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Interop.Tests
{

    public class PrimitiveGeometryTests
    {


        static private ILogger logger = NullLogger<PrimitiveGeometryTests>.Instance;
        static private ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());


        //[Fact]
        //public void can_build_composite_curve()
        //{
        //    using (var model = MemoryModel.OpenRead(@".\\TestFiles\Primitives\\composite_curve.ifc"))
        //    {
        //        var compCurve = model.Instances.OfType<IIfcCompositeCurve>().FirstOrDefault();
        //        compCurve);
        //        var face = geomEngine.CreateFace(compCurve);
        //        face.OuterBound);
        //    }
        //}
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void can_build_ifc_faceted_brep(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Primitives\ifc_faceted_brep.ifc"))
            {
                var shape = model.Instances.OfType<IIfcFacetedBrep>().FirstOrDefault();
                shape.Should().NotBeNull();
                var geomEngine = XbimGeometryEngine.CreateGeometryEngine(engineVersion, model, loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, logger);
                geom.Count.Should().Be(1);
                geom.First().Volume.Should().BeApproximately(3232.386, 5e-3);
            }
        }
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void can_build_closed_shell(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Primitives\faulty_closed_shell.ifc"))
            {
                var shape = model.Instances.OfType<IIfcClosedShell>().FirstOrDefault();
                shape.Should().NotBeNull();
                var geomEngine = XbimGeometryEngine.CreateGeometryEngine(engineVersion, model, loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, logger).FirstOrDefault();
                geom.Volume.Should().BeApproximately(-136033.82966702414, 1e-5);
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
                var geomEngine = XbimGeometryEngine.CreateGeometryEngine(engineVersion, model, loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, logger).FirstOrDefault();
                geom.Should().NotBeNull();
            }
            if (engineVersion == XGeometryEngineVersion.V6) Console.WriteLine("V6 produces a different result from V5, investigation is required");
        }


    }
}
