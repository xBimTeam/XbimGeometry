using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using System.Linq;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Interop.Tests
{
  
    // [DeploymentItem("TestFiles")]
    public class WireAndFaceTests
    {
       
        static private ILogger logger = NullLogger<WireAndFaceTests>.Instance;

        static private ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        
        [Fact]
        public void Empty_Polyline()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Polyline.ifc"))
            {
                var poly = model.Instances.OfType<IIfcPolyline>().FirstOrDefault();
                var geomEngine = new XbimGeometryEngine(model, loggerFactory);
                var wire = geomEngine.CreateWire(poly, logger);
            }
        }
        [Fact]
        public void Composite_curve_issue_261()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Composite_curve_issue_261.ifc"))
            {
                var composite_curve = model.Instances.OfType<IIfcCompositeCurve>().FirstOrDefault();
                var geomEngine = new XbimGeometryEngine(model, loggerFactory);
                var wire = geomEngine.CreateWire(composite_curve, logger);
            }
        }

        [Fact]
        public void TestIfFaceIsPlanar()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\non_planar_wire.ifc"))
            {
                var polyloop = model.Instances.OfType<IIfcPolyLoop>().FirstOrDefault();
                polyloop.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, loggerFactory);
                var face = geomEngine.CreateFace(polyloop, logger);
                face.OuterBound.Should().NotBeNull();
            }

        }

    }
}
