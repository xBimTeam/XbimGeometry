using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{
    public class CurveTests
    {


        private readonly MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        private readonly IXbimGeometryServicesFactory factory;
        private readonly ILoggerFactory _loggerFactory;

        public CurveTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            this.factory = factory;
            _loggerFactory = loggerFactory;
        }


        [Fact]

        public void Can_Get_Derivatives()
        {
            IXGeometryEngineV6 geomSvc = factory.CreateGeometryEngineV6(_dummyModel, _loggerFactory);
            var basisLine = IfcMoq.IfcLine3dMock( magnitude: 100,  origin: IfcMoq.IfcCartesianPoint3dMock(0, 0, 0), direction: IfcMoq.IfcDirection3dMock(0, 0, 1));
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve3dMock(basisCurve: basisLine, trimParam1: 1, trimParam2: 30);
            var curveFactory = geomSvc.CurveFactory;
            var tc = curveFactory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve

            var p1 = tc?.GetFirstDerivative(1,out var direction1);
            IXDirection? normal2 =null;
            var p2 = tc?.GetSecondDerivative(1, out var direction2, out normal2);
            normal2.Should().NotBeNull();
            double.IsNaN(normal2.X).Should().BeTrue();
            double.IsNaN(normal2.Y).Should().BeTrue();
            normal2.IsNull.Should().BeTrue();
        }
    }
}
