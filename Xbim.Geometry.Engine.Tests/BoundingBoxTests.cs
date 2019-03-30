using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;

namespace GeometryTests
{
    [TestClass]
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    public class BoundingBoxTests
    {
        [TestMethod]
        [DeploymentItem(@"Ifc2x3Files\PositionWithMappedItems.ifc", "Model")]
        public void TestMatchingGeometryPoints()
        {
            FileInfo f = new FileInfo(@"Model\PositionWithMappedItems.ifc");
            // var falseModel = CreateGeometry(f, false);
            var trueModel = CreateGeometry(f, false);

            using (var m = IfcStore.Open(trueModel.FullName))
            {
                using (var geomstore = m.GeometryStore)
                using (var geomReader = geomstore.BeginRead())
                {
                    foreach (var inst in geomReader.ShapeInstances)
                    {
                        // Debug.WriteLine(inst.IfcProductLabel + " " + inst.BoundingBox);
                        var b = inst.BoundingBox;
                        var tf = b.Transform(inst.Transformation);
                        Debug.WriteLine(inst.IfcProductLabel + " -  bb " + tf);

                        //Debug.WriteLine(inst.IfcProductLabel + " - " + inst.Transformation);
                    }

                    var allRegCollections = geomReader.ContextRegions;
                    // ReportAdd($"Region Collections count: {allRegCollections.Count}");
                    foreach (var regionCollection in allRegCollections)
                    {
                        // ReportAdd($"Region Collection (#{regionCollection.ContextLabel}) count: {regionCollection.Count}");
                        foreach (var r in regionCollection)
                        {
                            if (r.Population > 0)
                                Debug.WriteLine ("Region\t'" + r.Name + "'\t" + r.Population + "\t" + r.Size+ "\t" + r.Centre);
                        }
                    }
                }
            }
        }

        private static FileInfo CreateGeometry(FileInfo f, bool mode)
        {
            using (var m = IfcStore.Open(f.FullName))
            {
                var c = new Xbim3DModelContext(m);
                c.CreateContext(null, mode);
                var newName = Path.ChangeExtension(f.FullName, mode + ".xbim");
                m.SaveAs(newName);
                return new FileInfo(newName);
            }
        }
    }
}
