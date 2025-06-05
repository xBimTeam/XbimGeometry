using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    public struct Deflection
    {
        public double Linear;
        public double Angular;

        public Deflection(double linear, double angular) : this()
        {
            Linear = linear;
            Angular = angular;
        }
    }


    internal class DynamicDeflection : ICanLog, IDisposable
    {
        private readonly IModelFactors _modelFactors;
        private readonly IXbimGeometryEngine _engine;
        private readonly Dictionary<int, double> _curveLengthCache = new Dictionary<int, double>();
        private readonly Dictionary<int, (double, double)> _curveBoundsCache = new Dictionary<int, (double, double)>();

        public DynamicDeflection(IModelFactors modelFactors, IXbimGeometryEngine engine, ILogger logger) : base(logger)
        {
            _modelFactors = modelFactors ?? throw new ArgumentNullException(nameof(modelFactors));
            _engine = engine ?? throw new ArgumentNullException(nameof(engine));
        }

        public Deflection GetDeflection(
            IIfcGeometricRepresentationItem shape,
            XbimRect3D bbox,
            double defaultLinear,
            double defaultAngular,
            DynamicDeflectionSettings cfg)
        {
            if (shape is not IIfcSweptAreaSolid &&
                shape is not IIfcSweptDiskSolid)
                return new Deflection(defaultLinear, defaultAngular);

            var sectionWidth = 0.0;
            var sectionHeight = 0.0;
            var sweepLength = 0.0;

            if (shape is IIfcSweptAreaSolid areaSolid)
            {
                var bounds = GetSectionBounds(areaSolid.SweptArea);
                if (!bounds.HasValue)
                    return new(defaultLinear, defaultAngular);

                (sectionWidth, sectionHeight) = bounds.Value;
                sweepLength = GetSweepingLength(areaSolid, bbox);
            }
            else if (shape is IIfcSweptDiskSolid diskSolid)
            {
                sectionWidth = sectionHeight = diskSolid.Radius * 2;
                sweepLength = GetDiskSweepLength(diskSolid, bbox);
            }


            var minDim = Math.Min(sectionWidth, sectionHeight);
            var slenderness = sweepLength / minDim;

            if(slenderness < cfg.CriticalSlenderness)
                return new Deflection(defaultLinear, defaultAngular);

            double targetFacetCount = 0;

            // Use custom strategy if available
            if (cfg.CustomStrategy != null)
                targetFacetCount = cfg.CustomStrategy.GetTargetFacets(minDim / _modelFactors.OneMilliMeter, slenderness);

            if(targetFacetCount == 0)
            {
                var threshold = _modelFactors.OneMilliMeter * cfg.BaselineSectionWidthMm;
                var sizeRatio = minDim / threshold;
                targetFacetCount = cfg.MinimumPerimeterFacets * sizeRatio;
            }

            targetFacetCount = Clamp(targetFacetCount, cfg.MinimumPerimeterFacets, cfg.MaximumPerimeterFacets);
            
            var angular = 4 * Math.PI / targetFacetCount;
            var effectiveRadius = Math.Min(sectionWidth, sectionHeight) / 2;
            var linear = effectiveRadius * (1 - Math.Cos(angular / 2));
            
            var maxLinearDeflection = effectiveRadius * cfg.MaxLinearDeflectionRatio;
            linear = Math.Min(linear, maxLinearDeflection);
            angular = Math.Min(angular, cfg.MaxAngularDeflectionRadians);

            // take the maximum to avoid surpassing the defaule level of detail of the model and produce overly-fined meshes
            return new(Math.Max(linear, defaultLinear), Math.Max(angular, defaultAngular));
        }

        private double Clamp(double value, double min, double max)
        {
            return value < min ? min : (value > max ? max : value);
        }

        private double GetDiskSweepLength(IIfcSweptDiskSolid disk, XbimRect3D bbox)
        {
            if (disk.StartParam.HasValue && disk.EndParam.HasValue)
            {
                return Math.Abs(disk.EndParam.Value - disk.StartParam.Value);
            }
            return CurveLength(disk.Directrix);
        }

        private double GetSweepingLength(IIfcSweptAreaSolid solid, XbimRect3D boundingBox)
        {
            if (solid == null) return 0.0;

            if (solid is IIfcExtrudedAreaSolid extruded)
                return Math.Abs(extruded.Depth);

            if (solid is IIfcRevolvedAreaSolid revolved)
                return ComputeRevolvedSweepLength(revolved);

            if (solid is IIfcSurfaceCurveSweptAreaSolid surfaceSweep)
            {
                if (surfaceSweep.StartParam.HasValue && surfaceSweep.EndParam.HasValue)
                {
                    return Math.Abs(surfaceSweep.EndParam.Value - surfaceSweep.StartParam.Value);
                }

                return CurveLength(surfaceSweep.Directrix);
            }

            if (solid is IIfcFixedReferenceSweptAreaSolid fixedSweep)
            {
                if (fixedSweep.StartParam.HasValue && fixedSweep.EndParam.HasValue)
                {
                    return Math.Abs(fixedSweep.EndParam.Value - fixedSweep.StartParam.Value);
                }

                return CurveLength(fixedSweep.Directrix);
            }

            return Math.Sqrt(boundingBox.SizeX * boundingBox.SizeX +
                             boundingBox.SizeY * boundingBox.SizeY +
                             boundingBox.SizeZ * boundingBox.SizeZ);
        }

        private double CurveLength(IIfcCurve curve)
        {
            try
            {
                if (_curveLengthCache.TryGetValue(curve.EntityLabel, out var len))
                    return len;

                using (var geom = _engine.CreateCurve(curve, _logger))
                {
                    if (geom != null && geom.IsValid)
                    {
                        return _curveLengthCache[curve.EntityLabel] = geom.Length;
                    }
                }
            }
            catch (Exception ex)
            {
                LogWarning(ex, curve, "Failed to compute curve length for deflection calculation");
            }

            return 0;
        }

        private (double width, double height)? GetSectionBounds(IIfcProfileDef profile)
        {
            try
            {
                switch (profile)
                {
                    case IIfcRectangleProfileDef rect:
                        return (rect.XDim, rect.YDim);

                    case IIfcCircleProfileDef circle:
                        var diameter = circle.Radius * 2;
                        return (diameter, diameter);

                    case IIfcEllipseProfileDef ellipse:
                        return (ellipse.SemiAxis1 * 2, ellipse.SemiAxis2 * 2);

                    case IIfcIShapeProfileDef iShape:
                        return (iShape.OverallWidth, iShape.OverallDepth);

                    case IIfcLShapeProfileDef lShape:
                        return (lShape.Width ?? 0, lShape.Depth);

                    case IIfcTShapeProfileDef tShape:
                        return (tShape.FlangeWidth, tShape.Depth);

                    case IIfcUShapeProfileDef uShape:
                        return (uShape.FlangeWidth, uShape.Depth);

                    case IIfcCShapeProfileDef cShape:
                        return (cShape.Width, cShape.Depth);

                    case IIfcArbitraryProfileDefWithVoids arbitraryWithVoids:
                        return GetCurveBounds(arbitraryWithVoids.OuterCurve);

                    case IIfcArbitraryClosedProfileDef arbitrary:
                        return GetCurveBounds(arbitrary.OuterCurve);

                    default:
                        return null;
                }
            }
            catch (Exception ex)
            {
                LogWarning(ex, profile, "Failed to determine profile bounds for deflection calculation");
                return null;
            }
        }

        private (double width, double height)? GetCurveBounds(IIfcCurve curve)
        {
            try
            {

                if (_curveBoundsCache.TryGetValue(curve.EntityLabel, out var bb))
                    return bb;

                using (var geom = _engine.Create(curve, _logger))
                {
                    if (geom != null && geom.IsValid)
                    {
                        var bounds = geom.BoundingBox;
                        bb = (bounds.SizeX, bounds.SizeY);
                        return _curveBoundsCache[curve.EntityLabel] = bb;
                    }
                }
            }
            catch (Exception ex)
            {
                LogWarning(ex, curve, "Failed to compute curve bounds for deflection calculation");
            }

            return null;
        }

        private static double ComputeRevolvedSweepLength(IIfcRevolvedAreaSolid revolved)
        {
            if (revolved == null) return 0.0;
            var angle = Math.Abs(revolved.Angle);
            if (angle <= 0.0) return 0.0;

            var axis = revolved.Axis;
            var axisLoc = axis?.Location?.Coordinates;
            var axisDirArr = axis?.Axis?.DirectionRatios;
            var ax = (double)axisLoc[0].Value;
            var ay = (double)axisLoc[1].Value;
            var az = (double)axisLoc[2].Value;

            var dx = (double)axisDirArr[0].Value;
            var dy = (double)axisDirArr[1].Value;
            var dz = (double)axisDirArr[2].Value;

            var dirLen = Math.Sqrt(dx * dx + dy * dy + dz * dz);
            if (dirLen == 0) { dx = 0; dy = 0; dz = 1; dirLen = 1; }
            dx /= dirLen; dy /= dirLen; dz /= dirLen;

            double px = 0, py = 0, pz = 0;

            if (revolved.SweptArea is IIfcParameterizedProfileDef pProf &&
                pProf.Position?.Location?.Coordinates is { } pc)
            {
                px = (double)pc[0].Value;
                py = (double)pc[1].Value;
            }

            var vx = px - ax;
            var vy = py - ay;
            var vz = pz - az;

            var dot = vx * dx + vy * dy + vz * dz;
            var projX = dot * dx;
            var projY = dot * dy;
            var projZ = dot * dz;

            var perpX = vx - projX;
            var perpY = vy - projY;
            var perpZ = vz - projZ;

            var radius = Math.Sqrt(perpX * perpX + perpY * perpY + perpZ * perpZ);

            return radius * angle;           // model units
        }

        public void Dispose()
        {
            _curveLengthCache.Clear();
            _curveBoundsCache.Clear();
        }
    }
}
