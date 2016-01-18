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
                var advancedBrep = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(advancedBrep);
                var basin = _xbimGeometryCreator.CreateSolid(advancedBrep);
                Assert.IsTrue(basin.Volume>0);
            }
        }

        [TestMethod]
        public void TriangulatedFaceSetTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\BasinTessellation.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var basin = _xbimGeometryCreator.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue(basin.BoundingBox.Volume > 23913891);
                Assert.IsTrue(basin.BoundingBox.Volume < 23913893);
            }
        }

        [TestMethod]
        public void ExtrudedSolidWithNullPositionTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\Wall.ifc"))
            {
                var extSolid = model.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault();
                Assert.IsNotNull(extSolid);
                var wall = _xbimGeometryCreator.CreateSolid(extSolid);
                Assert.IsTrue(wall.Volume > 0);
                
            }
        }
        [TestMethod]
        public void BasinBRepTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\BasinBRep.ifc"))
            {
                var brep = model.Instances.OfType<IfcFacetedBrep>().FirstOrDefault();
                Assert.IsNotNull(brep);
                var basin = _xbimGeometryCreator.CreateSolid(brep);
                Assert.IsTrue(basin.Volume > 0);

            }
        }
        [TestMethod]
        public void CsgSolidTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\Bath.ifc"))
            {
                var csgSolid = model.Instances.OfType<IfcCsgSolid>().FirstOrDefault();
                Assert.IsNotNull(csgSolid);
                var bath = _xbimGeometryCreator.CreateSolid(csgSolid);
                Assert.IsTrue(bath.Volume > 0);

            }
        }
    }
}
