using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.NetCore.Tests
{
    public class ProfileFactoryTests
    {
        #region Setup

        static ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole().SetMinimumLevel(LogLevel.Information));
        static MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        static IXModelGeometryService _modelSvc = XbimGeometryEngine.CreateModelGeometryService(_dummyModel, loggerFactory);
        #endregion

        [Fact]
        void Can_Build_IIfcArbitraryProfileDef_With_Composite_Curve_Void()
        {
            using var model = MemoryModel.OpenRead("testfiles/ArbritaryClosedProfileWithCompositeCurveVoid.ifc");
            var engine = XbimGeometryEngine.CreateGeometryEngineV6(model, loggerFactory);
            var ifcArbitraryProfileDefWithVoids = model.Instances[1] as IIfcArbitraryProfileDefWithVoids;
            var v6face = engine.ProfileFactory.BuildFace(ifcArbitraryProfileDefWithVoids);
            Assert.NotNull(v6face);
            var v5Face = engine.CreateFace(ifcArbitraryProfileDefWithVoids);
            Assert.NotNull(v5Face);
            v5Face.Area.Should().BeApproximately(v6face.Area, 1e-5);
        }

        /// <summary>
        /// In this test several of the inner voids are not areas
        /// </summary>
        [Fact]
        void Can_Build_IIfcArbitraryProfileDef_With_bad_precision_on_closing_segments()
        {
            using var model = MemoryModel.OpenRead("testfiles/ArbritaryClosedProfileWithBadPrecisionOnClosingSegments.ifc");
            var engine = XbimGeometryEngine.CreateGeometryEngineV6(model, loggerFactory);
            var ifcArbitraryProfileDefWithVoids = model.Instances[1] as IIfcArbitraryProfileDefWithVoids;
            var v6face = engine.ProfileFactory.BuildFace(ifcArbitraryProfileDefWithVoids);
            Assert.NotNull(v6face);
            var v5Face = engine.CreateFace(ifcArbitraryProfileDefWithVoids);
            Assert.NotNull(v5Face);
            v5Face.Area.Should().BeApproximately(v6face.Area, 1e-5);
        }
    }
}
