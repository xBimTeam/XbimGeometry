using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3;
using Xbim.Ifc4x3.GeometricModelResource;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.Ifc4x3.MeasureResource;
using Xbim.Ifc4x3.ProfileResource;
using Xbim.IO.Memory;
using Xunit;
using ILoggerFactory = Microsoft.Extensions.Logging.ILoggerFactory;



namespace Xbim.Geometry.Engine.Tests.IFC4x3Tests
{
    public class IfcSectionedSurfaceTests
    {
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;
        private const double Tolerance = 1e-5;


        public IfcSectionedSurfaceTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }



        [Fact]
        public void CanBuildIfcSectionedSurface()
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildIfcSectionedSurface));
            var surface = BuildIfcSectionedSurface(model);
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            using (var stream = File.Create("0000000000000000000000000000.ifc"))
            {
                model.SaveAsStep21(stream);
            }


            // Act
            var xSurface = modelSvc.SurfaceFactory.Build(surface);

            // Assert
            xSurface.Should().NotBeNull();
        }



        private static IfcSectionedSurface CreateIfcSectionedSurfaceWithOpenProfiles(
            IfcCurve directrix,
            List<IfcOpenCrossProfileDef> crossSections,
            List<IfcAxis2PlacementLinear> sectionPositions,
            MemoryModel model)
        {
            if (crossSections.Count != sectionPositions.Count)
            {
                throw new ArgumentException("Cross-sections count must match section positions count.");
            }

            // Create the IfcSectionedSurface instance
            var sectionedSurface = model.Instances.New<IfcSectionedSurface>(surface =>
            {
                surface.Directrix = directrix;
                surface.CrossSections.AddRange(crossSections);
                surface.CrossSectionPositions.AddRange(sectionPositions);
            });

            return sectionedSurface;
        }

        private static IfcOpenCrossProfileDef CreateOpenCrossProfileWithWidthsAndSlopes(
                 MemoryModel model,
                 List<double> widths,
                 List<double> slopes,
                 bool horizontalWidths,
                 IfcCartesianPoint offsetPoint = null,
                 List<string> tags = null)
        {
            if (widths.Count != slopes.Count)
            {
                throw new ArgumentException("Widths and slopes must have the same number of elements.");
            }

            var openProfile = model.Instances.New<IfcOpenCrossProfileDef>(profile =>
            {
                profile.ProfileName = "OpenCrossProfile";
                profile.ProfileType = Ifc4x3.ProfileResource.IfcProfileTypeEnum.CURVE;
                profile.HorizontalWidths = horizontalWidths;

                foreach (var width in widths)
                {
                    profile.Widths.Add(new IfcNonNegativeLengthMeasure(width));
                }

                foreach (var slope in slopes)
                {
                    profile.Slopes.Add(new IfcPlaneAngleMeasure(slope));
                }

                if (tags != null)
                {
                    foreach (var tag in tags)
                    {
                        profile.Tags.Add(new IfcLabel(tag));
                    }
                }
                if (offsetPoint != null)
                {
                    profile.OffsetPoint = offsetPoint;
                }
            });

            return openProfile;
        }

        private static IfcCartesianPoint CreateOffsetPoint(MemoryModel model, double x, double y)
        {
            return model.Instances.New<IfcCartesianPoint>(p =>
            {
                p.X = x;
                p.Y = y;
            });
        }

        private static IfcLine CreateDirectrix(MemoryModel model)
        {
            var start = model.Instances.New<IfcCartesianPoint>(p => { p.X = 0; p.Y = 0; p.Z = 0; });
            var direction = model.Instances.New<IfcVector>(d =>
            {

                d.Orientation = model.Instances.New<IfcDirection>(d => { d.X = 0; d.Y = 0; d.Z = 1; });
                d.Magnitude = 1;
            });

            var line = model.Instances.New<IfcLine>(l =>
            {
                l.Pnt = start;
                l.Dir = direction;
            });

            return line;
        }


        private static IfcAxis2PlacementLinear CreateSectionPosition
                (double radius, double semiCircleEndAngle, IfcCurveMeasureSelect distanceAlong, MemoryModel model)

        {
            var pointByDistanceExpression = model.Instances.New<IfcAxis2PlacementLinear>(placement =>
            {
                placement.Location = CreatePointByDistanceExpression(radius, semiCircleEndAngle, distanceAlong, model);
                placement.RefDirection = model.Instances.New<IfcDirection>(refDir =>
                {
                    refDir.X = 1;
                    refDir.Y = 0;
                    refDir.Z = 0;
                });
                placement.Axis = model.Instances.New<IfcDirection>(dir =>
                {
                    dir.X = 0;
                    dir.Y = 1;
                    dir.Z = 0;
                });

            });
            return pointByDistanceExpression;
        }

        private static IfcPointByDistanceExpression CreatePointByDistanceExpression
                  (double radius, double semiCircleEndAngle, IfcCurveMeasureSelect distanceAlong, MemoryModel model)
        {
            var pointByDistanceExpression = model.Instances.New<IfcPointByDistanceExpression>(point =>
            {
                point.DistanceAlong = distanceAlong;
                point.BasisCurve = model.Instances.New<IfcTrimmedCurve>(curve =>
                {
                    curve.BasisCurve = model.Instances.New<IfcCircle>(circle =>
                    {
                        circle.Radius = radius;
                        circle.Position = model.Instances.New<IfcAxis2Placement3D>(placement =>
                        {

                            placement.Axis = model.Instances.New<IfcDirection>(dir =>
                            {
                                dir.X = 0;
                                dir.Y = 0;
                                dir.Z = 1;
                            });
                            placement.RefDirection = model.Instances.New<IfcDirection>(refDir =>
                            {
                                refDir.X = 0;
                                refDir.Y = 1;
                                refDir.Z = 0;
                            });
                            placement.Location = model.Instances.New<IfcCartesianPoint>(p =>
                            {
                                p.X = 0;
                                p.Y = 0;
                                p.Z = 0;
                            });
                        });
                    });
                    curve.MasterRepresentation = Ifc4x3.GeometryResource.IfcTrimmingPreference.PARAMETER;
                    curve.SenseAgreement = true;
                    curve.Trim1.Add(new IfcParameterValue(0));
                    curve.Trim2.Add(new IfcParameterValue(semiCircleEndAngle));
                });
            });
            return pointByDistanceExpression;
        }

        public static IfcSectionedSurface BuildIfcSectionedSurface(MemoryModel model)
        {
            var crossSections = new List<IfcOpenCrossProfileDef>();
            var random = new Random();

            for (var i = 0; i < 5; i++)
            {

                if (i == 0)
                {
                    var widths = new List<double> { 2, 3, 2 };
                    var slopes = new List<double> { 0.5, 0, -0.5 };

                    var openCrossProfile = CreateOpenCrossProfileWithWidthsAndSlopes(
                        model,
                        widths,
                        slopes,
                        horizontalWidths: true,
                        tags: new List<string> { "P1", "P2", "P3", "P4" });
                    crossSections.Add(openCrossProfile);

                    widths = new List<double> { 3.5, 3.5 };
                    slopes = new List<double> { 0.5, -0.5 };

                    openCrossProfile = CreateOpenCrossProfileWithWidthsAndSlopes(
                        model,
                        widths,
                        slopes,
                        horizontalWidths: true,
                        tags: new List<string> { "P1", "P2", "P4" });
                    crossSections.Add(openCrossProfile);

                    openCrossProfile = CreateOpenCrossProfileWithWidthsAndSlopes(
                        model,
                        widths,
                        slopes,
                        horizontalWidths: true,
                        tags: new List<string> { "P1", "P2", "P4" });
                    crossSections.Add(openCrossProfile);
                }
                else if (i == 1)
                {
                    var widths = new List<double> { 3.5, 0, 3.5 };
                    var slopes = new List<double> { 0.5, 0, -0.5 };

                    var openCrossProfile = CreateOpenCrossProfileWithWidthsAndSlopes(
                        model,
                        widths,
                        slopes,
                        horizontalWidths: true,
                        tags: new List<string> { "P1", "P2", "P3", "P4" });
                    crossSections.Add(openCrossProfile);
                }
                else
                {
                    var widths = new List<double> { 2.0, i < 3 ? 3 + random.Next(2) : 1 + random.Next(2), 2 };
                    var slopes = new List<double> { 0.5, 0, -0.5 };

                    var openCrossProfile = CreateOpenCrossProfileWithWidthsAndSlopes(
                        model,
                        widths,
                        slopes,
                        horizontalWidths: true,
                        tags: new List<string> { "P1", "P2", "P3", "P4" });
                    crossSections.Add(openCrossProfile);

                }


            }

            var sectionPositions = new List<IfcAxis2PlacementLinear>
            {
                CreateSectionPosition(100 , 90, new IfcLengthMeasure(0.0), model),
                CreateSectionPosition(100 , 90, new IfcLengthMeasure(10.0), model),
                CreateSectionPosition(100 , 180, new IfcLengthMeasure(20.0), model),
                CreateSectionPosition(100 , 180, new IfcLengthMeasure(40.0), model),
                CreateSectionPosition(100 , 180, new IfcLengthMeasure(50.0), model),
                CreateSectionPosition(100 , 180, new IfcLengthMeasure(60.0), model),
                CreateSectionPosition(100 , 180, new IfcLengthMeasure(70.0), model),
            };

            var directrix = CreateDirectrix(model);

            return CreateIfcSectionedSurfaceWithOpenProfiles(directrix, crossSections, sectionPositions, model);

        }

    }

}
