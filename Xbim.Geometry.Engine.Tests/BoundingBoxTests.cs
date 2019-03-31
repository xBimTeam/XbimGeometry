using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;
using Xbim.Temp.Scene;

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

        [TestMethod]
        [DeploymentItem(@"Ifc4TestFiles\grid-placement.ifc")]
        public void GeometryInstanceBoundingBoxesMatchForGridPlacement()
        {
            var fileName = "grid-placement.ifc";
            GeometryInstanceBoundingBoxMatchesOn(fileName);
        }


        [TestMethod]
        [DeploymentItem(@"SolidTestFiles\OneWallOneVoid.ifc")]
        public void GeometryInstanceBoundingBoxesMatchSingleVoidedWall()
        {
            var fileName = "OneWallOneVoid.ifc";
            GeometryInstanceBoundingBoxMatchesOn(fileName);
        }

        [TestMethod]
        [DeploymentItem(@"ifc2x3Files\positionWithMappedItems.ifc")]
        public void GeometryInstanceBoundingBoxesMatchWithMaps()
        {
            var fileName = "positionWithMappedItems.ifc";
            GeometryInstanceBoundingBoxMatchesOn(fileName);
        }

        [TestMethod]
        [DeploymentItem(@"SolidTestFiles\OneWallTwoWindows.ifc")]
        public void GeometryInstanceBoundingBoxesMatch()
        {
            var fileName = "OneWallTwoWindows.ifc";
            GeometryInstanceBoundingBoxMatchesOn(fileName);
        }

        private void GeometryInstanceBoundingBoxMatchesOn(string fileName)
        {
            FileInfo f = new FileInfo(fileName);
            var xbimName = CreateGeometry(f, true).FullName;

            using (var model = IfcStore.Open(xbimName))
            {
                using (var geomStore = model.GeometryStore)
                using (var geomReader = geomStore.BeginRead())
                {

                    var shapeInstances = geomReader.ShapeInstances;
                    var FailedShapes = new List<string>();
                    var FailedInstances = new List<string>();


                    foreach (var shapeInstance in shapeInstances.OrderBy(x => x.IfcProductLabel))
                    {
                        IXbimShapeGeometryData shapeGeom = geomReader.ShapeGeometry(shapeInstance.ShapeGeometryLabel);
                        if (shapeGeom.Format != (byte)XbimGeometryType.PolyhedronBinary)
                            continue;

                        var xbimMesher = new XbimMesher();
                        xbimMesher.AddMesh(shapeGeom.ShapeData);

                        XbimRect3D computedBox = new XbimRect3D();
                        foreach (var pnt in xbimMesher.Positions)
                        {
                            computedBox.Union(pnt);
                        }
                        // Debug.WriteLine("Min: " + computedBox.Min);
                        XbimRect3D storedBox = XbimRect3D.FromArray(shapeGeom.BoundingBox);

                        var instanceBox = shapeInstance.BoundingBox;
                        var locatedBox = storedBox.Transform(shapeInstance.Transformation);
                        if (!BoxesAreSame(instanceBox, locatedBox, model.ModelFactors.Precision))
                        {
                            FailedInstances.Add(shapeInstance.ToString());
                            if (FailedInstances.Count > 10)
                                continue; // exit the loop if many errors.
                        }

                        if (!BoxesAreSame(computedBox, storedBox, model.ModelFactors.Precision))
                        {
                            FailedShapes.Add(shapeGeom.ToString());
                            if (FailedShapes.Count > 10)
                                continue; // exit the loop if many errors.
                        }
                    }

                    Assert.AreEqual(0, FailedShapes.Count, "Shapes failing bounding box test: " + string.Join(",\r\n", FailedShapes));
                    Assert.AreEqual(0, FailedInstances.Count, "Instances failing bounding box test: " + string.Join(",\r\n", FailedInstances));
                }
            }
        }

        private bool BoxesAreSame(XbimRect3D computedBox, XbimRect3D storedBox, double precision)
        {
            var minSame = pointIsSame(computedBox.Min, storedBox.Min, precision);
            var maxSame = pointIsSame(computedBox.Max, storedBox.Max, precision);

            return minSame & maxSame;
        }

        private bool pointIsSame(XbimPoint3D pt1, XbimPoint3D pt2, double precision)
        {
            var x = CoordIsSame(pt1.X, pt2.X, precision);
            var y = CoordIsSame(pt1.Y, pt2.Y, precision);
            var z = CoordIsSame(pt1.Z, pt2.Z, precision);

            return x && y && z;
        }

        private bool CoordIsSame(double z1, double z2, double precision)
        {
            var delta = Math.Abs(z1 - z2);
            return  delta < 10 * precision;
        }
    }
}
