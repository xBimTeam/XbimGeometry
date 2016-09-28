using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"Ifc4TestFiles\", "Ifc4TestFiles")]
    [TestClass]
    public class AdvBrepRecursionTests
    {
        [TestMethod]
        public void CreateAdvancedBrep()
        {
            var engine = new XbimGeometryEngine();
            var files = Directory.GetFiles(@"Ifc4TestFiles\NBSAdvancedBreps", "*.ifc");
            foreach (var file in files)
            {
                using (var store = IfcStore.Open(file))
                {

                    foreach (var brep in store.Instances.OfType<IIfcAdvancedBrep>())
                    {
                        var solid = engine.CreateSolid(brep);

                        Assert.IsTrue(solid.Volume>0,"Breps should have a positive volume");
                    }
                }
            }
        }       
    }
}
