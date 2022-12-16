using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.NetCore.Tests
{
    public class GeometryFactoryTests
    {
        #region Setup

        static ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        static MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        static IXModelGeometryService _modelSvc = XbimGeometryEngine.CreateModelGeometryService(_dummyModel, loggerFactory);

        #endregion

        [Fact]
        public void Can_Build_IfcCartesianTransformationOperator3D()
        {
            var ct3d = IfcMoq.IfcCartesianTransformationOperator3DMoq();
            var ax3 = IfcMoq.IfcAxis2Placement3DMock();
            IXLocation loc;
            IXMatrix mat;
            _modelSvc.GeometryFactory.BuildMapTransform(ct3d, ax3, out loc, out mat);
            loc.Should().NotBeNull();
            mat.Should().NotBeNull();

        }

    }

}
