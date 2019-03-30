using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Ifc;
using Xbim.Temp.Scene;

namespace GeometryTests
{
    [TestClass]
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    public class SceneBoundingBoxes
    {
        [TestMethod]
        [DeploymentItem(@"SolidTestFiles\OneWallTwoWindows.ifc")]
        public void TestBoundingBoxes()
        {
            DirectoryInfo d = new DirectoryInfo(".");
            Debug.WriteLine(d.FullName);
            var xbimName = MeshFile("OneWallTwoWindows.ifc");

            using (var model = IfcStore.Open(xbimName))
            {
                using (var geomStore = model.GeometryStore)
                using (var geomReader = geomStore.BeginRead())
                {

                    var shapeInstances = geomReader.ShapeInstances.Where(s => (s.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded
                            ||
                            s.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsOnly && s.IfcTypeId == 498 // openings
                            )
                            );

                    foreach (var shapeInstance in shapeInstances.OrderBy(x => x.IfcProductLabel))
                    {
                        IXbimShapeGeometryData shapeGeom = geomReader.ShapeGeometry(shapeInstance.ShapeGeometryLabel);
                        if (shapeGeom.Format != (byte)XbimGeometryType.PolyhedronBinary)
                            continue;

                        var xbimMesher = new XbimMesher();
                        xbimMesher.AddMesh(shapeGeom.ShapeData);
                        var p1 = xbimMesher.Positions;

                        XbimRect3D r = new XbimRect3D();
                        foreach (var pnt in p1)
                        {
                            r.Union(pnt);
                        }
                        Debug.WriteLine("Min: " + r.Min);

                        // XbimRect3D newRec = XbimRect3D.FromArray(shapeGeom);
                    }
                }
            }
        }

        private string MeshFile(string v)
        {
            string newName = Path.ChangeExtension(v, "xbim");
            using (var model = IfcStore.Open(v))
            {
                Xbim.ModelGeometry.Scene.Xbim3DModelContext c = new Xbim.ModelGeometry.Scene.Xbim3DModelContext(model);
                c.CreateContext();
                model.SaveAs(newName);
            }
            return newName;
        }
    }
}
