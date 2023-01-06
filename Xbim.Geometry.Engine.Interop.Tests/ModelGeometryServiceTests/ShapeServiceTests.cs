using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.NetCore.Tests
{

    public class ShapeServiceTests
    {
        #region Setup

        static ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        static MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        static IXModelGeometryService _modelSvc = XbimGeometryEngine.CreateModelGeometryService(_dummyModel, loggerFactory);
        private IXShapeService _shapeService;

        #endregion


        public ShapeServiceTests(IXShapeService shapeService)
        {
            _shapeService = shapeService;
        }

        [Fact]
        public void CanScaleShape()
        {
            var solidService = _modelSvc.SolidFactory;
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(startParam: 0, endParam: 100, radius: 30, innerRadius: 15);
            var solid = (IXSolid)solidService.Build(ifcSweptDisk);
            var scale = 0.001;

            Assert.False(solid.IsEmptyShape());
            Assert.True(solid.IsValidShape());

            var moved = _shapeService.Scaled(solid, scale);

            moved.Should().NotBeNull();
            var volDiff = Math.Abs(moved.As<IXSolid>().Volume - solid.Volume * Math.Pow(scale, 3));
            volDiff.Should().BeLessThan(Math.Pow(scale, 3));
        }
    }

}

