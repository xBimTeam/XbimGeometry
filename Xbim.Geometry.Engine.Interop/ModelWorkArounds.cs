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
        //this bug exists in ifc files exported by Revit up to releases 20.1 
        public const string RevitIncorrectBsplineSweptCurve = "#RevitIncorrectBsplineSweptCurve";
        //this bug exists in all current Revit exported Ifc files
        public const string RevitIncorrectArcCentreSweptCurve = "#RevitIncorrectArcCentreSweptCurve";
        //this bug exists in all current Revit exported Ifc files
        public const string RevitSweptSurfaceExtrusionInFeet = "#RevitSweptSurfaceExtrusionInFeet";
        public const string PolylineTrimLengthOneForEntireLine = "#PolylineTrimLengthOneForEntireLine";
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
                return ; //nothing to do
            var matches = Regex.Matches(header.FileName.OriginatingSystem, revitPattern, RegexOptions.IgnoreCase);
            var matchesAltUI = Regex.Matches(header.FileName.OriginatingSystem, revitAltUIPattern, RegexOptions.IgnoreCase);
            string version=null;
            if (matches.Count > 0 && matches[0].Groups.Count == 2)
                version = matches[0].Groups[1].Value;
            else if(matchesAltUI.Count > 0 && matchesAltUI[0].Groups.Count == 2)
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
    }
}
