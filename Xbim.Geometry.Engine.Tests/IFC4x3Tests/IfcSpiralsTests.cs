using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3;
using Xbim.Ifc4x3.GeometricModelResource;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests.IFC4x3Tests
{
    public class IfcSpiralsTests
    {
        private const double Precision = 1e-5;
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;


        public IfcSpiralsTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }

        [Theory]
        [InlineData(-207.019667802706, SegmentType.ClothoidCurve_300_1000, -142.857142857143, 100)]
        [InlineData(173.205080756888, SegmentType.ClothoidCurve_neg300_negInf, -100, 100)]
        public void CanBuildClothoid(double clothoidConstant, SegmentType type, double firstParam, double length)
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildSecondOrderPolynomialSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var clothoid = model.Instances.New<IfcClothoid>(p =>
            {
                p.ClothoidConstant = clothoidConstant;
                p.Position = model.Instances.New<IfcAxis2Placement2D>(placement =>
                {
                    placement.Location = model.Instances.New<IfcCartesianPoint>(pnt =>
                    {
                        pnt.X = 0;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });

                    placement.RefDirection = model.Instances.New<IfcDirection>(pnt =>
                    {
                        pnt.X = 1;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });
                });
            });


            // Act
            var xClothoid = modelSvc.CurveFactory.BuildSpiral(clothoid, firstParam, firstParam + length);

            // Assert
            xClothoid.Should().NotBeNull();

            foreach (var location in CurveSegmentData.GetPoints(type))
            {
                var point = xClothoid.GetPoint(location.Key + xClothoid.FirstParameter);
                point.X.Should().BeApproximately(location.Value.X, Precision);
                point.Y.Should().BeApproximately(location.Value.Y, Precision);
            }
        }


        [Fact]
        public void CanBuildSecondOrderPolynomialSpiral()
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildSecondOrderPolynomialSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var secondOrderPolynomialSpiral = model.Instances.New<IfcSecondOrderPolynomialSpiral>(p =>
            {
                p.LinearTerm = 125;
                p.ConstantTerm = -625;
                p.QuadraticTerm = -146.200886910643;
                p.Position = model.Instances.New<IfcAxis2Placement2D>(placement =>
                {
                    placement.Location = model.Instances.New<IfcCartesianPoint>(pnt =>
                    {
                        pnt.X = 0;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });

                    placement.RefDirection = model.Instances.New<IfcDirection>(pnt =>
                    {
                        pnt.X = 1;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });
                });
            });

            // Act
            var xSpiral = modelSvc.CurveFactory.BuildSpiral(secondOrderPolynomialSpiral, -100, 100);

            // Assert
            xSpiral.Should().NotBeNull();
        }


        [Theory]
        [InlineData(-110.668191970032, 100, SegmentType.BlossCurve_inf_300)]
        public void CanBuildThirdOrderPolynomialSpiral(double cubicTerm, double quadraticTerm, SegmentType spiralType)
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildSecondOrderPolynomialSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var polynomialSpiral = model.Instances.New<IfcThirdOrderPolynomialSpiral>(p =>
            {
                p.CubicTerm = -110.668191970032;
                p.QuadraticTerm = 100;
                p.Position = model.Instances.New<IfcAxis2Placement2D>(placement =>
                {
                    placement.Location = model.Instances.New<IfcCartesianPoint>(pnt =>
                    {
                        pnt.X = 0;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });

                    placement.RefDirection = model.Instances.New<IfcDirection>(pnt =>
                    {
                        pnt.X = 1;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });
                });
            });

            // Act
            var xSpiral = modelSvc.CurveFactory.BuildSpiral(polynomialSpiral, 0, 100);

            // Assert
            xSpiral.Should().NotBeNull();
            foreach (var location in CurveSegmentData.GetPoints(spiralType))
            {
                var point = xSpiral.GetPoint(location.Key);
                point.X.Should().BeApproximately(location.Value.X, Precision);
                point.Y.Should().BeApproximately(location.Value.Y, Precision);
            }
        }


        [Theory]
        [InlineData(82.48484305114, -67.097076273516, 61.2742234216927, -68.9807356362507, -91.7493208218373, 141.521951256265, null, 300.0, SegmentType.VienneseBend_300_1000)]
        [InlineData(78.8880838459446, -63.7638813456506, 57.7378785242934, -64.2314061308743, -83.922298125931, 125.657906854859, null, 300.0, SegmentType.VienneseBend_300_inf)]
        [InlineData(78.8880838459446, -63.7638813456506, 57.7378785242934, -64.2314061308743, -83.922298125931, 125.657906854859, null, null, SegmentType.VienneseBend_negInf_neg300)]
        public void CanBuildSeventhOrderPolynomialSpiral
            (double septicTerm, double? sexticTerm, double? quinticTerm, 
            double? quarticTerm, double? cubicTerm, double? quadraticTerm, 
            double? linearTerm, double? constantTerm, SegmentType spiralType)
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildSecondOrderPolynomialSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var polynomialSpiral = model.Instances.New<IfcSeventhOrderPolynomialSpiral>(p =>
            {
                p.SepticTerm = septicTerm;
                p.SexticTerm = sexticTerm;
                p.QuinticTerm = quinticTerm;
                p.QuarticTerm = quarticTerm;
                p.CubicTerm = cubicTerm;
                p.QuadraticTerm = quadraticTerm;
                p.LinearTerm = linearTerm;
                p.ConstantTerm = constantTerm;

                p.Position = model.Instances.New<IfcAxis2Placement2D>(placement =>
                {
                    placement.Location = model.Instances.New<IfcCartesianPoint>(pnt =>
                    {
                        pnt.X = 0;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });

                    placement.RefDirection = model.Instances.New<IfcDirection>(pnt =>
                    {
                        pnt.X = 1;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });
                });
            });

            // Act
            var xSpiral = modelSvc.CurveFactory.BuildSpiral(polynomialSpiral, 0, 100);

            // Assert
            xSpiral.Should().NotBeNull();
            foreach (var location in CurveSegmentData.GetPoints(spiralType))
            {
                var point = xSpiral.GetPoint(location.Key);
                point.X.Should().BeApproximately(location.Value.X, Precision);
                point.Y.Should().BeApproximately(location.Value.Y, Precision);
            }
        }


        [Theory]
        [InlineData(-1884.95559215388, 173.205080756888, 0, SegmentType.SineCurve_inf_300)]
        [InlineData(-2692.79370307697, 207.019667802706, 1000, SegmentType.SineCurve_1000_300)]
        [InlineData(-1884.95559215388, 173.205080756888, -300, SegmentType.SineCurve_neg300_negInf)]
        public void CanBuildSineSpiral(double sineTerm, double linearTerm, double constantTerm, SegmentType spiralType)
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildSineSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var sineSpiral = model.Instances.New<IfcSineSpiral>(p =>
            {
                p.SineTerm = sineTerm; 
                p.LinearTerm = linearTerm;
                p.ConstantTerm = constantTerm;

                p.Position = model.Instances.New<IfcAxis2Placement2D>(placement =>
                {
                    placement.Location = model.Instances.New<IfcCartesianPoint>(pnt =>
                    {
                        pnt.X = 0;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });

                    placement.RefDirection = model.Instances.New<IfcDirection>(pnt =>
                    {
                        pnt.X = 1;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });
                });
            });

            // Act
            var xSpiral = modelSvc.CurveFactory.BuildSpiral(sineSpiral, 0, 100);

            // Assert
            xSpiral.Should().NotBeNull();
            foreach (var location in CurveSegmentData.GetPoints(spiralType))
            {
                var point = xSpiral.GetPoint(location.Key);
                point.X.Should().BeApproximately(location.Value.X, Precision);
                point.Y.Should().BeApproximately(location.Value.Y, Precision);
            }
        }


        [Theory]
        [InlineData(857.142857142857, -461.538461538462, SegmentType.CosineCurve_neg1000_neg300)]
        [InlineData(-600, -600, SegmentType.CosineCurve_neg300_negInf)]
        public void CanBuildCosineSpiral(double cosineTerm, double constantTerm, SegmentType spiralType)
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildCosineSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var cosineSpiral = model.Instances.New<IfcCosineSpiral>(p =>
            {
                p.CosineTerm = cosineTerm;
                p.ConstantTerm = constantTerm;

                p.Position = model.Instances.New<IfcAxis2Placement2D>(placement =>
                {
                    placement.Location = model.Instances.New<IfcCartesianPoint>(pnt =>
                    {
                        pnt.X = 0;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });

                    placement.RefDirection = model.Instances.New<IfcDirection>(pnt =>
                    {
                        pnt.X = 1;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });
                });
            });

            // Act
            var xSpiral = modelSvc.CurveFactory.BuildSpiral(cosineSpiral, 0, 100);

            // Assert
            xSpiral.Should().NotBeNull();
            foreach (var location in CurveSegmentData.GetPoints(spiralType))
            {
                var point = xSpiral.GetPoint(location.Key);
                point.X.Should().BeApproximately(location.Value.X, Precision);
                point.Y.Should().BeApproximately(location.Value.Y, Precision);
            }
        }
    } 
}
