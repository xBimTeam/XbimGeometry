using System;
using System.Linq;
using System.Text.RegularExpressions;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;
using Microsoft.Extensions.Logging;

namespace Xbim.Geometry.Engine.Interop
{
    /// <summary>
    /// Extension methods for <see cref="IModel"/>s applying workarounds for known issues in varuous authoring tool versions
    /// </summary>
    public static class ModelWorkArounds
    {
        //this bug exists in ifc files exported by Revit up to releases 20.1 
        const string RevitIncorrectBsplineSweptCurve = "#RevitIncorrectBsplineSweptCurve";
        //this bug exists in all current Revit exported Ifc files
        const string RevitIncorrectArcCentreSweptCurve = "#RevitIncorrectArcCentreSweptCurve";
        //this bug exists in all current Revit exported Ifc files
        const string RevitSweptSurfaceExtrusionInFeet = "#RevitSweptSurfaceExtrusionInFeet";
        const string PolylineTrimLengthOneForEntireLine = "#PolylineTrimLengthOneForEntireLine";

        // Incorrect precision specified in Archicad models,
        // we make the model more precise since Archicad is casual about it.
        const string ArchicadPrecisionWorkaround = "#ArchicadPrecisionWorkaround";

        // we do not have an implementation of composite curves used for 3d placement, 
        // this woraround replaces the curve in 2d point by distance entity with a gradient curve
        const string NotImplemented2DPointByDistanceWorkaround = "#NotImplemented2DPointByDistanceWorkaround";

        /// <summary>
        /// Adds ArchiCAD specific workarounds
        /// </summary>
        /// <param name="model"></param>
        /// <param name="_logger"></param>
        public static void AddArchicadWorkArounds(this IModel model, Microsoft.Extensions.Logging.ILogger _logger)
        {
            var header = model.Header;
            var modelFactors = model.ModelFactors as XbimModelFactors;
            string byPattern = @"by Graphisoft ArchiCAD";

            if (header.FileName == null || string.IsNullOrWhiteSpace(header.FileName.OriginatingSystem))
                return; //nothing to do
            var matches = Regex.Matches(header.FileName.OriginatingSystem, byPattern, RegexOptions.IgnoreCase);
            if (matches.Count > 0) //looks like Archicad
            {
                if (modelFactors.ApplyWorkAround(ArchicadPrecisionWorkaround)) // if the workadound has been added then the precision is already been changed.
                    return;
                var evaluatingPrecision = modelFactors.Precision;
                // faces of IFCCLOSEDSHELLs should be larger than precision
                foreach (IIfcClosedShell shell in model.Instances.OfType<IIfcClosedShell>())
                {
                    foreach (var face in shell.CfsFaces)
                    {
                        foreach (var faceBound in face.Bounds)
                        {
                            var lpPrec = GetPrec(faceBound.Bound, evaluatingPrecision);
                            if (lpPrec)
                            {
                                // we apply the workaround and exit
                                _logger.LogWarning("Added ArchicadPrecisionWorkaround for #{ifcEntityLabel}.", faceBound.Bound.EntityLabel);
                                modelFactors.Precision /= 100;
                                modelFactors.AddWorkAround(ArchicadPrecisionWorkaround);
                                return;
                            }
                        }
                    }
                }
            }
        }

        private static bool GetPrec(IIfcLoop bound, double precision)
        {
            if (bound is IIfcPolyLoop pl)
            {
                var dropped = 0;
                var count = pl.Polygon.Count();
                var last = pl.Polygon[count - 1]; // start from last, we measure all distances closing the loop
                XbimPoint3D prevPoint = new XbimPoint3D(last.X, last.Y, last.Z);
                for (int i = 0; i < count; i++)
                {
                    XbimPoint3D thisPoint = new XbimPoint3D(pl.Polygon[i].X, pl.Polygon[i].Y, pl.Polygon[i].Z);
                    var dist = (prevPoint - thisPoint).Length;
                    if (dist < precision)
                    {
                        dropped++;
                    }
                    prevPoint = thisPoint;
                }
                if (count - dropped < 3)
                    return true;
            }
            return false;
        }

        /// <summary>
        /// Adds a work around for versions of the Revit exporter prior to and inclusing Version(17, 4, 0, 0);
        /// This did not correctly align linear extrusions bounds and surface due to a missing placement value
        /// </summary>
        /// <param name="model"></param>
        /// <returns>The lookup tag in ModelFactors.WorkArounds for the workaround or null if not required due to a later version of the exporter</returns>
        public static void AddRevitWorkArounds(this IModel model)
        {
            //it looks like all revit exports up to the 2020 release do not consider the local placement, so broadening the previous catch
            var header = model.Header;
            var modelFactors = model.ModelFactors as XbimModelFactors;
            //typical pattern for the revit exporter
            string revitPattern = @"- Exporter (\d*.\d*.\d*.\d*)";
            string revitAltUIPattern = @"- Exporter- Alternate UI (\d*.\d*.\d*.\d*)";
            if (header.FileName == null || string.IsNullOrWhiteSpace(header.FileName.OriginatingSystem))
                return; //nothing to do
            var matches = Regex.Matches(header.FileName.OriginatingSystem, revitPattern, RegexOptions.IgnoreCase);
            var matchesAltUI = Regex.Matches(header.FileName.OriginatingSystem, revitAltUIPattern, RegexOptions.IgnoreCase);
            string version = null;
            if (matches.Count > 0 && matches[0].Groups.Count == 2)
                version = matches[0].Groups[1].Value;
            else if (matchesAltUI.Count > 0 && matchesAltUI[0].Groups.Count == 2)
                version = matchesAltUI[0].Groups[1].Value;
            if (matches.Count > 0 || matchesAltUI.Count > 0) //looks like Revit
            {
                //SurfaceOfLinearExtrusion bug found in all current versions, comment this code out when it is fixed               
                modelFactors.AddWorkAround(RevitIncorrectArcCentreSweptCurve);
                modelFactors.AddWorkAround(RevitSweptSurfaceExtrusionInFeet);
                if (!string.IsNullOrEmpty(version)) //we have the build versions
                {
                    if (Version.TryParse(version, out Version modelVersion))
                    {
                        //uncomment this code when it is fixed in the exporter
                        ////SurfaceOfLinearExtrusion bug found in version 20.1.0 and earlier
                        //var revitIncorrectArcCentreSweptCurveVersion = new Version(20, 1, 0, 0);
                        //if (modelVersion <= revitIncorrectArcCentreSweptCurveVersion)
                        //{
                        //    modelFactors.AddWorkAround(RevitIncorrectArcCentreSweptCurve);
                        //}
                        //SurfaceOfLinearExtrusion bug found in version 20.0.0 and earlier
                        var revitIncorrectBsplineSweptCurveVersion = new Version(20, 0, 0, 500);
                        if (modelVersion <= revitIncorrectBsplineSweptCurveVersion)
                        {
                            modelFactors.AddWorkAround(RevitIncorrectBsplineSweptCurve);
                        }
                    }

                }
            }
        }
        /// <summary>
        /// In some processors the directrix of a sweep has a trimmed polyline, where the upper trim parameter has been set to 1
        /// The publisher incorrectly intended to mean the entire length of the Polyline
        /// The Polyline should be defined as the sum of the lengths of all of the segments, and partial segments required
        /// </summary>
        /// <param name="model"></param>
        /// <returns></returns>
        public static string AddWorkAroundTrimForPolylinesIncorrectlySetToOneForEntireCurve(this IModel model)
        {
            var header = model.Header;
            var modelFactors = model.ModelFactors as XbimModelFactors;
            modelFactors.AddWorkAround(PolylineTrimLengthOneForEntireLine);
            return PolylineTrimLengthOneForEntireLine;
        }

        /// <summary>
        /// Because xbim does not implement some cases for ifcpointbydistanceexpression we adjust the ifc to replace those scenarios 
        /// with one that can be calculated
        /// </summary>
        /// <param name="model">The model to fix</param>
        /// <param name="logger"></param>
        public static bool AddNotImplemented2DPointByDistanceWorkaround(this IModel model, ILogger logger)
        {
            // we need to replace IfcCompositeCurve but not their subclasses
            //
            var curvesToReplace = model.Instances.OfType<Ifc4x3.GeometryResource.IfcPointByDistanceExpression>()
                .Where(x =>
                    x.BasisCurve.GetType().Name == nameof(Xbim.Ifc4x3.GeometryResource.IfcCompositeCurve) // testing by name to avoid subclasses
                    ).Select(x => x.BasisCurve.EntityLabel).Distinct().ToList();
            if (!curvesToReplace.Any())
                return false;
            var modelFactors = model.ModelFactors as XbimModelFactors;
            if (modelFactors is null)
            {
                logger.LogWarning("Could not evaluated ArchicadPrecisionWorkaround.");
                return false;
            }
            if (modelFactors.ApplyWorkAround(NotImplemented2DPointByDistanceWorkaround)) // if the workadound has been added then the precision is already been changed.
                return false;
            logger.LogInformation("Applying NotImplemented2DPointByDistance Workaround.");
            using (var txn = model.BeginTransaction("Create new entities"))
            {
                foreach (var curveId in curvesToReplace)
                {
                    var curve = model.Instances[curveId] as Xbim.Ifc4x3.GeometryResource.IfcCompositeCurve;
                    if (curve is null)
                        continue;
                    var relavantPoints = model.Instances.OfType<Ifc4x3.GeometryResource.IfcPointByDistanceExpression>().Where(x => x.BasisCurve.EntityLabel == curveId).ToList();
                    var maxDistanceObj = relavantPoints.Select(x => x.DistanceAlong).Max(x => x.Value);
                    if (maxDistanceObj is not double mxD)
                    {
                        logger.LogError("Invalid max distance when NotImplemented2DPointByDistance Workaround on {entityLabel}.", curveId);
                        continue;
                    }
                    // var maxParameterObj = model.Instances.OfType<IfcPointByDistanceExpression>().Where(x => x.DistanceAlong is IfcParameterValue).Select(x => x.DistanceAlong).Max(x => x.Value);
                    // we create an horizontal line segment that covers the entire lenght of the gradient curve (maxDistance)
                    var zeroPoint = model.Instances.New<Xbim.Ifc4x3.GeometryResource.IfcCartesianPoint>((x) => x.SetXY(0, 0));
                    var xDirection = model.Instances.New<Xbim.Ifc4x3.GeometryResource.IfcDirection>((x) => x.SetXY(1, 0));
                    var xVector = model.Instances.New<Xbim.Ifc4x3.GeometryResource.IfcVector>((x) =>
                    {
                        x.Orientation = xDirection;
                        x.Magnitude = new Ifc4x3.MeasureResource.IfcLengthMeasure(1);
                    });
                    var horizontalLine = model.Instances.New<Xbim.Ifc4x3.GeometryResource.IfcLine>((x) =>
                    {
                        x.Pnt = zeroPoint;
                        x.Dir = xVector;
                    });

                    var segmentPlacement = model.Instances.New<Xbim.Ifc4x3.GeometryResource.IfcAxis2Placement2D>((x) => x.Location = zeroPoint); // direction defaults to 1.0, 0.0 according to specs
                    var curveSegment = model.Instances.New<Ifc4x3.GeometryResource.IfcCurveSegment>((x) =>
                    {
                        x.Transition = Ifc4x3.GeometryResource.IfcTransitionCode.CONTSAMEGRADIENTSAMECURVATURE;
                        x.SegmentStart = new Ifc4x3.MeasureResource.IfcLengthMeasure(0);
                        x.SegmentLength = new Ifc4x3.MeasureResource.IfcLengthMeasure(mxD + modelFactors.OneMeter);
                        x.Placement = segmentPlacement;
                        x.ParentCurve = horizontalLine;
                    });
                    var gradient = model.Instances.New<Ifc4x3.GeometryResource.IfcGradientCurve>((x) =>
                    {
                        x.Segments.Add(curveSegment);
                        x.SelfIntersect = false;
                        x.BaseCurve = curve;
                    });

                    relavantPoints.ForEach(x => x.BasisCurve = gradient);
                }
                txn.Commit();
                modelFactors.AddWorkAround(NotImplemented2DPointByDistanceWorkaround);
            }
            return true;
        }
    }
}
