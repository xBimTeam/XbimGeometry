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
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;


        public IfcSpiralsTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }

        [Fact]
        public void CanBuildClothoid()
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildSecondOrderPolynomialSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var clothoid = model.Instances.New<IfcClothoid>(p =>
            {
                p.ClothoidConstant = 20;
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
            var xClothoid = modelSvc.CurveFactory.BuildSpiral(clothoid, -200, 200);

            // Assert
            xClothoid.Should().NotBeNull();
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


        [Fact]
        public void CanBuildThirdOrderPolynomialSpiral()
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildSecondOrderPolynomialSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var polynomialSpiral = model.Instances.New<IfcThirdOrderPolynomialSpiral>(p =>
            {
                //-120.989673502444, 112.624788044361, $, 1000
                p.ConstantTerm = 1000;
                p.QuadraticTerm = 112.624788044361;
                p.CubicTerm = -120.989673502444;
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
            var xSpiral = modelSvc.CurveFactory.BuildSpiral(polynomialSpiral, -100, 100);

            // Assert
            xSpiral.Should().NotBeNull();
        }

        [Fact]
        public void CanBuildSeventhOrderPolynomialSpiral()
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildSecondOrderPolynomialSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var polynomialSpiral = model.Instances.New<IfcSeventhOrderPolynomialSpiral>(p =>
            {
                p.ConstantTerm = 30;
                p.QuadraticTerm = -20;
                p.CubicTerm = 20;
                p.SepticTerm = 100;
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
            var xSpiral = modelSvc.CurveFactory.BuildSpiral(polynomialSpiral, -100, 100);

            // Assert
            xSpiral.Should().NotBeNull();
        }

        [Fact]
        public void CanBuildSineSpiral()
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildSineSpiral));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var sineSpiral = model.Instances.New<IfcSineSpiral>(p =>
            {
                p.SineTerm = -1884.95559215388;
                p.LinearTerm = 173.205080756888;
                //p.ConstantTerm = 0.3;

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
        }



        [Fact]
        public void IFCRailRoomSample()
        {
            using var model = MemoryModel.OpenRead(@"TestFiles\IFC4x3\GENERATED__Bloss_100.0_inf_300_1_Meter.ifc");
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);
            var spiral = model.Instances[46] as IfcThirdOrderPolynomialSpiral;
            var xSpiral = modelSvc.CurveFactory.BuildSpiral(spiral, 0, 100);

            // Todo: compare x,y and curvature against computed values
        }
    }
}
