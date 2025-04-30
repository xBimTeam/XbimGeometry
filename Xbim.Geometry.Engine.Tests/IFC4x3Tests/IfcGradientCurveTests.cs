using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.Threading.Tasks.Sources;
using Xbim.Common;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4x3;
using Xbim.Ifc4x3.GeometricConstraintResource;
using Xbim.Ifc4x3.GeometricModelResource;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.Ifc4x3.MeasureResource;
using Xbim.Ifc4x3.ProfileResource;
using Xbim.Ifc4x3.RepresentationResource;
using Xbim.IO.Memory;
using Xunit;
using ILoggerFactory = Microsoft.Extensions.Logging.ILoggerFactory;



namespace Xbim.Geometry.Engine.Tests.IFC4x3Tests
{
    public class IfcGradientCurveTests
    {
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;


        public IfcGradientCurveTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }

        [Theory]
        [InlineData(@"TestFiles\IFC4x3\test.ifc")]
        [InlineData(@"TestFiles\IFC4x3\Viadotto Acerno.ifc")]
        [InlineData(@"TestFiles\IFC4x3\SectionedSolidHorizontal-1.ifc")]
        // [InlineData(@"TestFiles\IFC4x3\PlacmentOfSignal.ifc")] // this has a known issue (discontinuous curve)
        [InlineData(@"TestFiles\IFC4x3\T2.ifc")]
        [InlineData(@"TestFiles\IFC4x3\T2-mod.ifc")]
        [InlineData(@"TestFiles\IFC4x3\T2-mod2.ifc")]
        public void CanBuildLinearPlacements(string filePath)
        {
            var logger = _loggerFactory.CreateLogger<IfcGradientCurveTests>();
            using var model = MemoryModel.OpenRead(filePath);
            model.AddNotImplemented2DPointByDistanceWorkaround(logger);

            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);
            var linPlacements = model.Instances.OfType<IfcLinearPlacement>().Take(10);
            foreach (var linPlacement in linPlacements)
            {
                var loc = modelSvc.ModelPlacementBuilder.BuildLocation(linPlacement, false);
                logger.LogDebug($"Linear placement {linPlacement.EntityLabel} has translation {Display(loc.Translation)} and rotation {Display(loc.Rotation)}");
            }
        }

        private string Display(IXPoint translation)
        {
            return $"X: {translation.X}, Y: {translation.Y}, Z: {translation.Z}";
        }

        private string Display(IXQuaternion quat)
        {
            return $"X: {quat.X}, Y: {quat.Y}, Z: {quat.Z}, W: {quat.W}";
        }

        [Theory]
        [InlineData(@"TestFiles\IFC4x3\test.ifc")]
        [InlineData(@"TestFiles\IFC4x3\Viadotto Acerno.ifc")]
        [InlineData(@"TestFiles\IFC4x3\SectionedSolidHorizontal-1.ifc")]
        [InlineData(@"TestFiles\IFC4x3\PlacmentOfSignal.ifc")]
        public void CanBuildIfcGradientCurve(string filePath)
        {
            // Arrange
            using var model = MemoryModel.OpenRead(filePath);
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);
            var curves = model.Instances.OfType<IfcGradientCurve>();

            foreach (var curve in curves)
            {
                // Act
                var xCurve = modelSvc.CurveFactory.Build(curve);

                // Assert
                xCurve.Should().NotBeNull();

                // further tests, if the curve is used in a placement, we want to check that the placement is valid
                var referencing = GetReferencingEntity(curve);
                if (referencing is IfcLinearPlacement plc)
                {
                    var loc = modelSvc.ModelPlacementBuilder.BuildLocation(plc, false);
                }
            }
        }

        private IPersistEntity? GetReferencingEntity(IPersistEntity item)
        {
            // part of IFCGRADIENTCURVE
            // or IFCPOINTBYDISTANCEEXPRESSION
            IPersistEntity? grad = null;
            if (item is IfcGradientCurve gc)
            {
                grad = gc;
            }
            else if (item is IfcCompositeCurve)
            {
                grad = item.Model.Instances.OfType<IfcGradientCurve>().FirstOrDefault(x => x.BaseCurve.EntityLabel == item.EntityLabel);
            }
            if (grad is null)
                return null;

            var point = item.Model.Instances.OfType<IfcPointByDistanceExpression>().FirstOrDefault(x => x.BasisCurve.EntityLabel == grad.EntityLabel);
            if (point == null)
            {
                var rep = item.Model.Instances.OfType<IfcShapeRepresentation>().FirstOrDefault(x => x.Items.Any(x => x.EntityLabel == grad.EntityLabel));
                if (rep is not null)
                    return rep;
                return null;
            }
            var medP = item.Model.Instances.OfType<IfcAxis2PlacementLinear>().FirstOrDefault(x => x.Location.EntityLabel == point.EntityLabel);
            if (medP == null)
                return null;
            var lp = item.Model.Instances.OfType<IfcLinearPlacement>().FirstOrDefault(x => x.RelativePlacement.EntityLabel == medP.EntityLabel);
            if (lp == null)
                return null;
            return lp;
        }
    }
}
