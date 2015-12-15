using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Ifc;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Common.Geometry;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"Ifc4TestFiles\", "Ifc4TestFiles")]
    [TestClass]
    public class Ifc4GeometryTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void AdvancedBrepTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\BasinAdvancedBrep.ifc"))
            {
                var count = model.Instances.Count;
                Assert.IsTrue(count== 147);
                var advancedBrep = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(advancedBrep);
                var basin = _xbimGeometryCreator.CreateSolid(advancedBrep);
                Assert.IsTrue(basin.Volume>0);
            }
        }
    }
}
