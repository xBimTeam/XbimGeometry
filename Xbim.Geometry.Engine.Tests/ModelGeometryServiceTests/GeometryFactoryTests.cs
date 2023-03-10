using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{
    public class GeometryFactoryTests
    {
        #region Setup


        private readonly MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        private readonly IXModelGeometryService _modelSvc;

        public GeometryFactoryTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _modelSvc = factory.CreateModelGeometryService(_dummyModel, loggerFactory);
        }

        #endregion

        [Fact]
        public void Can_Build_IfcCartesianTransformationOperator3D()
        {
            var ct3d = IfcMoq.IfcCartesianTransformationOperator3DMoq(3);
            var ax3 = IfcMoq.IfcAxis2Placement3DMock(loc: IfcMoq.IfcCartesianPoint3dMock(10, 5, 2));
            IXLocation location;
            IXMatrix matrix;
            _modelSvc.GeometryFactory.BuildMapTransform(ct3d, ax3, out location, out matrix);
            location.Should().NotBeNull();
            matrix.Should().NotBeNull();
            matrix.ScaleX.Should().Be(3);
            matrix.ScaleY.Should().Be(3);
            matrix.ScaleZ.Should().Be(3);
            location.Translation.X.Should().Be(-10);
            location.Translation.Y.Should().Be(-5);
            location.Translation.Z.Should().Be(-2);


        }


        [Fact]
        public void Can_Build_IfcCartesianTransformationOperator3DnonUniform()
        {
            double scale1 = 0.5, scale2 = 1.5, scale3 = 0.1;
            var ct3dNu = IfcMoq.IfcCartesianTransformationOperator3DMoqNonUniform(scale1, scale2, scale3);
            var ax3 = IfcMoq.IfcAxis2Placement3DMock(loc: IfcMoq.IfcCartesianPoint3dMock(10, 5, 2));
            IXLocation location;
            IXMatrix matrix;
            _modelSvc.GeometryFactory.BuildMapTransform(ct3dNu, ax3, out location, out matrix);
            location.Should().NotBeNull();
            matrix.Should().NotBeNull();
            matrix.ScaleX.Should().Be(scale1);
            matrix.ScaleY.Should().Be(scale2);
            matrix.ScaleZ.Should().Be(scale3);
            location.Translation.X.Should().Be(-10);
            location.Translation.Y.Should().Be(-5);
            location.Translation.Z.Should().Be(-2);
        }

    }

}
