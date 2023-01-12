using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.NetCore.Tests
{
    public class CurveTests
    {

        static ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        static MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
       

        [Fact]
        public void Can_Get_Derivatives()
        {
            IXGeometryEngineV6 geomSvc = XbimGeometryEngine.CreateGeometryEngineV6(_dummyModel, loggerFactory);
            var basisLine = IfcMoq.IfcLine3dMock( magnitude: 100,  origin: IfcMoq.IfcCartesianPoint3dMock(0, 0, 0), direction: IfcMoq.IfcDirection3dMock(0, 0, 1));
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve3dMock(basisCurve: basisLine, trimParam1: 1, trimParam2: 30);
            var curveFactory = geomSvc.CurveFactory;
            var tc = curveFactory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve

            var p1 = tc.GetFirstDerivative(1,out var direction1);
            var p2 = tc.GetSecondDerivative(1, out var direction2,out var normal2);

        }
    }
}
