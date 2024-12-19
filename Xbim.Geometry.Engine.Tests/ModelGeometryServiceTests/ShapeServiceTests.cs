using FluentAssertions;
using Microsoft.Extensions.Logging;
using Moq;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{

    public class ShapeServiceTests
    {
        private readonly MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        private readonly IXModelGeometryService _modelSvc;
        private readonly IXShapeService _shapeService;

        /// <summary>
        /// Initializes a new instance of the <see cref="ShapeServiceTests" /> class.
        /// </summary>
        /// <param name="shapeService">The shape service.</param>
        /// <param name="geometryServicesFactory">The geometry services factory.</param>
        /// <param name="loggerFactory">The logger factory.</param>
        public ShapeServiceTests(IXShapeService shapeService,
                                IXbimGeometryServicesFactory geometryServicesFactory,
                                ILoggerFactory loggerFactory)
        {
            _shapeService = shapeService;
            _modelSvc = geometryServicesFactory.CreateModelGeometryService(_dummyModel, loggerFactory);
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


        [Fact]
        public void CanUnionShapes()
        {
            var solidService = _modelSvc.SolidFactory;
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(startParam: 0, endParam: 60, radius: 30, innerRadius: 15);
            var ifcSweptDisk2 = IfcMoq.IfcSweptDiskSolidMoq(startParam: 50, endParam: 100, radius: 20, innerRadius:  5);
            var solid1 = (IXSolid)solidService.Build(ifcSweptDisk);
            var solid2 = (IXSolid)solidService.Build(ifcSweptDisk2);

            var unioinShape = _shapeService.Union(solid1, solid2, _modelSvc.Precision);

            unioinShape.Should().NotBeNull();
        }


        [Fact]
        public void CanCutShapes()
        {
            var solidService = _modelSvc.SolidFactory;
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(startParam: 0, endParam: 60, radius: 30, innerRadius: 15);
            var ifcSweptDisk2 = IfcMoq.IfcSweptDiskSolidMoq(startParam: 50, endParam: 100, radius: 30, innerRadius: 15);
            var solid1 = (IXSolid)solidService.Build(ifcSweptDisk);
            var solid2 = (IXSolid)solidService.Build(ifcSweptDisk2);

            var cutShape = _shapeService.Cut(solid1, solid2, _modelSvc.Precision);

            cutShape.Should().NotBeNull();
        }


        [Fact]
        public void CanIntersectShapes()
        {
            var solidService = _modelSvc.SolidFactory;
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(startParam: 0, endParam: 60, radius: 30, innerRadius: 10);
            var block = IfcMoq.IfcBlockMoq();
            var solid1 = (IXSolid)solidService.Build(ifcSweptDisk);
            var solid2 = solidService.Build(block);

            var intersectShape = _shapeService.Intersect(solid1, solid2, _modelSvc.Precision);

            intersectShape.Should().NotBeNull();
        }


        [Theory]
        [MemberData(nameof(Transformations))]
        public void CanTransformShape(IXMatrix mat, double volFactor, IIfcBlock ifcBlock)
        {
            //Arrange
            var solidService = _modelSvc.SolidFactory;
            var solid = solidService.Build(ifcBlock);

            solid.IsEmptyShape().Should().BeFalse();
            solid.IsValidShape().Should().BeTrue();

            //Act
            var transformed = _shapeService.Transform(solid, mat) as IXSolid;

            //Assert
            transformed.Should().NotBeNull();
            transformed.IsEmptyShape().Should().BeFalse();
            transformed.IsValidShape().Should().BeTrue();
            transformed.Volume.Should().BeApproximately(solid.Volume * volFactor, 0.001);
        }


        [Fact]
        public void CanTransformShape_WithIfcTransformationOperator3D()
        {
            //Arrange
            double scale = 2.3;
            var transformationOperator = IfcMoq.IfcCartesianTransformationOperator3DMoq
                                                                                    (scale, 33, 0, 0);
            var mat = _modelSvc.GeometryFactory.BuildTransform(transformationOperator);
            var solidService = _modelSvc.SolidFactory;
            var ifcSweptDisk = IfcMoq.IfcSweptAreaSolidMoq();

            //Act
            var solid = solidService.Build(ifcSweptDisk) as IXSolid;


            //Assert
            var transformed = _shapeService.Transform(solid, mat) as IXSolid;
            transformed.Should().NotBeNull();
            transformed.IsEmptyShape().Should().BeFalse();
            transformed.IsValidShape().Should().BeTrue();
            transformed.Volume.Should().BeApproximately(solid.Volume * Math.Pow(scale, 3), 0.001);
        }


        [Fact]
        public void CanTransformShape_WithIfcTransformationOperator3DnonUniform()
        {
            //Arrange
            double scale1 = 0.5, scale2 = 1.5, scale3 = 0.1;
            var transformationOperator = IfcMoq.IfcCartesianTransformationOperator3DMoqNonUniform
                                                                                    (scale1, scale2, scale3);
            var mat = _modelSvc.GeometryFactory.BuildTransform(transformationOperator);
            var solidService = _modelSvc.SolidFactory;

            //Act
            var solid = solidService.Build(IfcMoq.IfcBlockMoq());

            //Assert
            var transformed = _shapeService.Transform(solid, mat) as IXSolid;
            transformed.Should().NotBeNull();
            transformed.IsEmptyShape().Should().BeFalse();
            transformed.IsValidShape().Should().BeTrue();
            transformed.Volume.Should().BeApproximately(solid.Volume * scale1 * scale2 * scale3, 0.001);
        }


        #region Helpers

        public static IEnumerable<object[]> Transformations
        {
            get
            {
                var result = new List<object[]>();

                var simpleTranslation = new double[] { 1, 0, 0, 2,
                                                       0, 1, 0, 3,
                                                       0, 0, 1, 1,
                                                       0, 0, 0, 1 };

                var angle = 30 * Math.PI / 180;
                var rotationAroundXWithTranslation = new double[]
                    { 1,  0,                0,                 2.1,
                      0,  Math.Cos(angle), -Math.Sin(angle),   7.3,
                      0,  Math.Sin(angle),  Math.Cos(angle),   4.7,
                      0,  0,                0,                 1    };

                var nonUniformScaling = new double[] { 1,   0,   0,   3,
                                                       0,   1,   0,   3,
                                                       0,   0,   1,   3,
                                                       0.5, 1.5, 0.1, 1 };

                // volume reduction factor = 1, the shape should be preserved
                result.Add(new object[] { ToMatrix(simpleTranslation), 1, IfcMoq.IfcBlockMoq() });

                result.Add(new object[] { ToMatrix(rotationAroundXWithTranslation), 1, IfcMoq.IfcBlockMoq() });

                // For non-uniformly scaled cubes, volume reduction factor = 0.5 x 1.5 x 0.1 = 0.075
                // as the volume has a linear relationship with the dimensions
                result.Add(new object[] { ToMatrix(nonUniformScaling), 0.075, IfcMoq.IfcBlockMoq(null, 10, 4, 3) });

                return result;
            }
        }

        private static IXMatrix ToMatrix(double[] values)
        {
            var matrix = new Mock<IXMatrix>();
            matrix.Setup(m => m.Values).Returns(values);
            matrix.Setup(m => m.M11).Returns(values[0]);
            matrix.Setup(m => m.M12).Returns(values[1]);
            matrix.Setup(m => m.M13).Returns(values[2]);

            matrix.Setup(m => m.M21).Returns(values[4]);
            matrix.Setup(m => m.M22).Returns(values[5]);
            matrix.Setup(m => m.M23).Returns(values[6]);

            matrix.Setup(m => m.M31).Returns(values[8]);
            matrix.Setup(m => m.M32).Returns(values[9]);
            matrix.Setup(m => m.M33).Returns(values[10]);

            matrix.Setup(m => m.M44).Returns(values[15]);

            matrix.Setup(m => m.ScaleX).Returns(values[12]);
            matrix.Setup(m => m.ScaleY).Returns(values[13]);
            matrix.Setup(m => m.ScaleZ).Returns(values[14]);

            matrix.Setup(m => m.OffsetX).Returns(values[3]);
            matrix.Setup(m => m.OffsetY).Returns(values[7]);
            matrix.Setup(m => m.OffsetZ).Returns(values[11]);

            return matrix.Object;
        }

        #endregion
    }

}

