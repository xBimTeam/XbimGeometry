using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;
using Xbim.Common.Geometry;
using System.Diagnostics;

namespace GeometryTests
{
    /// <summary>
    /// Summary description for PersistenceOfGeometry
    /// </summary>
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"PersistenceTestFiles\")]
    [TestClass]
    public class PersistenceOfGeometry
    {
        [TestMethod]
        public void TestPersistenceOfGrid()
        {
            int cntGrid = 0;
            int cntAll = 0;
            using (var model = IfcStore.Open("grid.ifc"))
            {
                var m3D = new Xbim3DModelContext(model);
                m3D.CreateContext();
                cntGrid = GetShapeInstanceCount(model, 73906);
                cntAll = getAllCount(model);
                model.SaveAs("grid.xbim");
            }

            using (var model = IfcStore.Open("grid.xbim"))
            {
                var cntGrid2 = GetShapeInstanceCount(model, 73906);
                var cntAll2 = getAllCount(model);
                Assert.AreEqual(cntAll, cntAll2);
                Assert.AreEqual(cntGrid, cntGrid2);
            }
        }

        private int getAllCount(IfcStore model)
        {
            var cnt = 0;

            using (var geomstore = model.GeometryStore)
            using (var geomReader = geomstore.BeginRead())
            {
                foreach (var item in geomReader.ShapeInstances)
                {
                    Debug.WriteLine(item);
                    cnt++;
                }
            }
            return cnt;
        }

        private static int GetShapeInstanceCount(IfcStore model, int itemId)
        {
            var cnt = 0;
            var item = model.Instances[itemId];

            using (var geomstore = model.GeometryStore)
            using (var geomReader = geomstore.BeginRead())
            {
                foreach (var shapeInstance in geomReader.ShapeInstancesOfEntity(item).Where(x => x.RepresentationType != XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded))
                {
                    cnt++;
                }
            }
            return cnt;
        }
    }
}