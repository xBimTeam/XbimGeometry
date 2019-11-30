using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Xbim.Common;

namespace Xbim.Geometry.Engine.Interop
{
    public static class ModelWorkArounds
    {
        public const string SurfaceOfLinearExtrusion = "#SurfaceOfLinearExtrusion";
        public const string PolylineTrimLengthOneForEntireLine = "#PolylineTrimLengthOneForEntireLine";
        /// <summary>
        /// Adds a work around for versions of the Revit exporter prior to and inclusing Version(17, 0, 416, 0);
        /// This did not correctly align linear extrusions bounds and surface due to a missing placement value
        /// </summary>
        /// <param name="model"></param>
        /// <returns>The lookup tag in ModelFactors.WorkArounds for the workaround or null if not required due to a later version of the exporter</returns>
        public static string AddWorkAroundSurfaceofLinearExtrusionForRevit(this IModel model)
        {
            
            var header = model.Header;
            var modelFactors = model.ModelFactors as XbimModelFactors;
            string revitPattern = @"- Exporter\s(\d*.\d*.\d*.\d*)";
            if (header.FileName == null || string.IsNullOrWhiteSpace(header.FileName.OriginatingSystem))
                return null; //nothing to do
            var matches = Regex.Matches(header.FileName.OriginatingSystem, revitPattern, RegexOptions.IgnoreCase);
            if (matches.Count > 0) //looks like Revit
            {
                if (matches[0].Groups.Count == 2) //we have the build versions
                {
                    if (Version.TryParse(matches[0].Groups[1].Value, out Version modelVersion))
                    {
                        //SurfaceOfLinearExtrusion bug found in version 17.2.0 and earlier
                        var surfaceOfLinearExtrusionVersion = new Version(17, 2, 0, 0);
                        if (modelVersion <= surfaceOfLinearExtrusionVersion)
                        {
                            modelFactors.AddWorkAround(SurfaceOfLinearExtrusion);
                            return SurfaceOfLinearExtrusion;
                        }
                    }

                }
            }
            return null;
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
    }
}
