using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;

namespace GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [TestClass]
    public class IoTests
    {
        [TestMethod]
        [DeploymentItem("Ifc4TestFiles/Column.ifc")]
        public void CanCreateContextOnEsentFileWithoutGeometryCache()
        {
            using (var m = IfcStore.Open("Column.ifc"))
            {
                m.SaveAs("column.xbim");
            }

            using (var m = IfcStore.Open("Column.xbim", accessMode: Xbim.IO.Esent.XbimDBAccess.ReadWrite))
            {
                var c = new Xbim3DModelContext(m);
                c.CreateContext();
            }
        }
    }
}
