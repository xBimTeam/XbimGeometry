using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
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

        IXModelGeometryService _modelSvc;
        private readonly ILoggerFactory _loggerFactory;
        #endregion

        public ProfileFactoryTests(ILoggerFactory loggerFactory)
        {
            _loggerFactory = loggerFactory;
            var _dummyModel = new MemoryModel(new EntityFactoryIfc4());
            _modelSvc = XbimGeometryEngine.CreateModelGeometryService(_dummyModel, _loggerFactory);
        }
        [Fact]
        void Can_Build_IIfcArbitraryProfileDef_With_Composite_Curve_Void()
        {
            using var model = MemoryModel.OpenRead("testfiles/ArbritaryClosedProfileWithCompositeCurveVoid.ifc");
            var engine = XbimGeometryEngine.CreateGeometryEngineV6(model, _loggerFactory);
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
            var engine = XbimGeometryEngine.CreateGeometryEngineV6(model, _loggerFactory);
            var ifcArbitraryProfileDefWithVoids = model.Instances[1] as IIfcArbitraryProfileDefWithVoids;
            var v6face = engine.ProfileFactory.BuildFace(ifcArbitraryProfileDefWithVoids);
            Assert.NotNull(v6face);
            var v5Face = engine.CreateFace(ifcArbitraryProfileDefWithVoids);
            Assert.NotNull(v5Face);
            v5Face.Area.Should().BeApproximately(v6face.Area, 1e-5);
        }

        [Theory]
        [InlineData(200, 100, 0)]
        [InlineData(200, 0, 1000)]
        public void Can_build_circle_profile_def(double outerRadius, double locX, double locY)
        {
            var profileFactory = _modelSvc.ProfileFactory;
            var circleProfileDef = IfcMoq.IfcCircleProfileDefMock(radius: outerRadius, position: IfcMoq.IfcAxis2Placement2DMock(IfcMoq.IfcDirection2dMock(0, 1), IfcMoq.IfcCartesianPoint2dMock(locX, locY)));
            var profileFace = profileFactory.BuildFace(circleProfileDef);
            profileFace.Area.Should().BeApproximately((Math.PI * Math.Pow(outerRadius, 2)), 1e-9);
        }
        [Theory]
        [InlineData(200, 10)]
        [InlineData(200, 0)] //should not fail
        public void Can_build_circle_hollow_profile_def(double outerRadius, double wallThickness)
        {
            var profileFactory = _modelSvc.ProfileFactory;
            var circleHollowProfileDef = IfcMoq.IfcCircleHollowProfileDefMock(radius: outerRadius, wallThickness: wallThickness);
            var profileFace = profileFactory.BuildFace(circleHollowProfileDef);
            double innerRadius;
            if (circleHollowProfileDef.WallThickness > 0)
                innerRadius = outerRadius - wallThickness;
            else
                innerRadius = 0;
            profileFace.Area.Should().BeApproximately((Math.PI * Math.Pow(outerRadius, 2) - (Math.PI * Math.Pow(innerRadius, 2))), 1e-9);
        }
    }
}
