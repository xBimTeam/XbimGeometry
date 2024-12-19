using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
using Xbim.Common.Geometry;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Geometry.Exceptions;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{
    public class ProfileFactoryTests
    {
        #region Setup

        IXModelGeometryService _modelSvc;
        private readonly ILoggerFactory _loggerFactory;
        private readonly IXbimGeometryServicesFactory factory;
        #endregion

        public ProfileFactoryTests(ILoggerFactory loggerFactory, IXbimGeometryServicesFactory factory)
        {
            _loggerFactory = loggerFactory;
            this.factory = factory;
            var _dummyModel = new MemoryModel(new EntityFactoryIfc4());
            _modelSvc = factory.CreateModelGeometryService(_dummyModel, _loggerFactory);
        }
        [Fact]
        void Can_Build_IIfcArbitraryProfileDef_With_Composite_Curve_Void()
        {
            using var model = MemoryModel.OpenRead("testfiles/ArbritaryClosedProfileWithCompositeCurveVoid.ifc");
            var engine = factory.CreateGeometryEngineV6(model, _loggerFactory);
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
            var engine = factory.CreateGeometryEngineV6(model, _loggerFactory);
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
            var face = profileFactory.BuildFace(circleHollowProfileDef);
            double innerRadius;
            if (circleHollowProfileDef.WallThickness > 0)
                innerRadius = outerRadius - wallThickness;
            else
                innerRadius = 0;
            face.Area.Should().BeApproximately((Math.PI * Math.Pow(outerRadius, 2) - (Math.PI * Math.Pow(innerRadius, 2))), 1e-9);
            //wires, edges and curves cannot be built out of multiple wires
            Assert.Throws<XbimGeometryFactoryException>(() => profileFactory.BuildWire(circleHollowProfileDef));
            Assert.Throws<XbimGeometryFactoryException>(() => profileFactory.BuildEdge(circleHollowProfileDef));
            Assert.Throws<XbimGeometryFactoryException>(() => profileFactory.BuildCurve(circleHollowProfileDef));
        }

        [Theory]
        [InlineData(500, 20, 90, 15707.96327)]
        [InlineData(500, 20, 180, 31415.92654)]
        [InlineData(500, 20, 359, 62657.32015)]
        [InlineData(500, 20, 360, 62831.85307)]
        public void Can_build_centre_line_profile_def(double radius, double thickness, double paramEnd, double area)
        {
            var centreLine = IfcMoq.IfcTrimmedCurve2dMock(IfcMoq.IfcCircle2dMock(radius: radius), 0, paramEnd);
            var profile = IfcMoq.IfcCenterLineProfileDefMock(centreLine, thickness);
            var profileFactory = _modelSvc.ProfileFactory;
            if (paramEnd == 360) //expect an exception to be thrown, the centre line must not be closed
            {
                Assert.Throws<XbimGeometryFactoryException>(() => profileFactory.BuildFace(profile));
            }
            else
            {
                var face = profileFactory.BuildFace(profile);
                face.Area.Should().BeApproximately(area, 1e-5);
                var wire = profileFactory.BuildWire(profile);
                var edge = profileFactory.BuildEdge(profile);
                var curve = profileFactory.BuildCurve(profile);
            }
        }

        [Fact]
        public void Can_Build_Extruded_CompositeProfileDef()
        {
            using var model = MemoryModel.OpenRead("testfiles/CuttingOpeningInCompositeProfileDefTest.ifc");
            var engineV6 = factory.CreateGeometryEngineV6(model, _loggerFactory);
            var extrusion = model.Instances[43] as IIfcExtrudedAreaSolid;
            extrusion.Should().NotBeNull();
            var occ = engineV6.Build(extrusion) as IXCompound;
            occ.Should().NotBeNull();
            occ.IsSolidsOnly.Should().BeTrue();
            var compositeProfile = (IIfcCompositeProfileDef)extrusion.SweptArea;
            occ.Solids.Count().Should().Be(compositeProfile.Profiles.Count);                                                               
            occ.Solids.Sum(s => ((IXSolid)s).Volume).Should().BeApproximately(12399283891, 1);
           
            //check the old engine returns the same result
            var engineV5 = factory.CreateGeometryEngineV5(model, _loggerFactory);
            var occV5 = engineV5.Create(extrusion) as IXbimSolidSet;
            occV5.Count().Should().Be(compositeProfile.Profiles.Count);
            occV5.Sum(s=>s.Volume).Should().BeApproximately(12399283891, 1);
        }

    }
}
