using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]  
    public class XbimGeometryToIfcTests
    {
        //private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        //[TestMethod]
        //public void XbimPointDictionaryTest()
        //{
           
        //    var a = _xbimGeometryCreator.CreatePoint(1.1, 3.333399999, 2.2, 0.0001);
        //    var b = _xbimGeometryCreator.CreatePoint(1.1, 3.333411111, 2.2, 0.0001);
        //    var c = _xbimGeometryCreator.CreatePoint(1.1, 3.3341111, 2.2, 0.0001);
        //    var d = _xbimGeometryCreator.CreatePoint(3.3341111, 1.1, 2.2, 0.0001);
        //    var pointSet = new ConcurrentDictionary<IXbimPoint, string>();
        //    Assert.IsTrue(pointSet.TryAdd(a, "a"));
        //    Assert.IsFalse(pointSet.TryAdd(b, "b"));
        //    Assert.IsTrue(pointSet.Count == 1, "Duplicate point added");
        //    Assert.IsTrue(pointSet.ContainsKey(b), "Point lookup failed");
        //    Assert.IsTrue(pointSet.TryAdd(c, "c"));
        //    Assert.IsTrue(pointSet.TryAdd(d, "d"));
        //    Assert.IsTrue(pointSet.ContainsKey(c), "Point lookup failed");
        //    Assert.IsTrue(pointSet.ContainsKey(d), "Point lookup failed");
        //    Assert.IsTrue(pointSet.Count == 3, "New point failed to be added");
        //}

        //[TestMethod]
        //public void ConvertIfcCylinderToBRepTest()
        //{
        //    using (var m = XbimModel.CreateTemporaryModel())
        //    {
        //        using (var txn = m.BeginTransaction())
        //        {
                    
        //            var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //            var solid = _xbimGeometryCreator.CreateSolid(cylinder);
        //            var brep = _xbimGeometryCreator.CreateFacetedBrep(m, solid);
        //            var solid2 = _xbimGeometryCreator.CreateSolidSet(brep);
                   
        //            txn.Commit();
        //            try
        //            {
        //                //Uncomment below to see the results in Ifc
        //                //m.SaveAs("brep.ifc", XbimStorageType.IFC);
        //            }
        //            catch (Exception)
        //            {

        //                Assert.IsTrue(false, "Failed to save the results to Ifc");
        //            }
        //            Assert.IsTrue(solid2.Count == 1, "Expected one solid");
        //            IfcCsgTests.GeneralTest(solid2.First);

        //            Assert.IsTrue(brep.Outer.CfsFaces.Count == solid2.First.Faces.Count, "Number of faces in round tripped solid is not the same");
        //        }
        //    }
        //}

        //[TestMethod]
       
        //public void ConvertIfcBlockToBRepTest()
        //{ 
        //    using (var m = XbimModel.CreateTemporaryModel())
        //    {
        //        using (var txn = m.BeginTransaction())
        //        {
                    

        //            var block = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    
        //            var solid = _xbimGeometryCreator.CreateSolid(block);
        //            var brep = _xbimGeometryCreator.CreateFacetedBrep(m, solid);
        //            var xrep = _xbimGeometryCreator.CreateSolidSet(brep);
        //            Assert.IsTrue(xrep.Count == 1, "Expected one solid");
        //            Assert.IsTrue(solid.Volume-xrep.First.Volume<=m.ModelFactors.Precision, "Volume of round tripped block is not the same");
        //            IfcCsgTests.GeneralTest(xrep.First);
        //        }
        //    }
        //}


        //[TestMethod]
        //public void ConvertIfcRectangleProfileToBRepTest()
        //{
        //    using (var m = IfcModelBuilder.CreateandInitModel())
        //    {
        //        using (var txn = m.BeginTransaction())
        //        {
        //            //add a shape
        //            //Create a Definition shape to hold the geometry
        //            var shape = m.Instances.New<IfcShapeRepresentation>();
        //            shape.ContextOfItems = m.IfcProject.ModelContext();
        //            shape.RepresentationType = "Brep";
        //            shape.RepresentationIdentifier = "Body";

        //            //Create a Product Definition and add the model geometry to the wall
        //            var rep = m.Instances.New<IfcProductDefinitionShape>();
        //            rep.Representations.Add(shape);
        //            var building = m.Instances.OfType<IfcBuilding>().FirstOrDefault();
        //            Assert.IsNotNull(building, "Failed to find Building");
        //            building.Representation = rep;

                    

        //            var block = IfcModelBuilder.MakeExtrudedAreaSolid(m, IfcModelBuilder.MakeRectangleProfileDef(m, 10000, 20000), 30000);
        //            var solid = _xbimGeometryCreator.CreateSolid(block);
        //            var brep = _xbimGeometryCreator.CreateFacetedBrep(m, solid);
        //            shape.Items.Add(brep);
        //            var xrep = _xbimGeometryCreator.CreateSolidSet(brep);

        //            txn.Commit();
        //            try
        //            {
        //                //Uncomment below to see the results in Ifc
        //                //m.SaveAs("brep.ifc", XbimStorageType.IFC);
        //            }
        //            catch (Exception)
        //            {

        //                Assert.IsTrue(false, "Failed to save the results to Ifc");
        //            }
        //            Assert.IsTrue(xrep.Count == 1, "Expected one solid");
        //            Assert.IsTrue(solid.Volume - xrep.First.Volume <= m.ModelFactors.Precision, "Volume of round tripped cylinder is not the same");
        //            IfcCsgTests.GeneralTest(xrep.First);
        //        }
        //    }
        //}

        ///// <summary>
        ///// Converts a RectangleHollowProfile in XbimSolid format to an IfcFacetedBrep ad back again 
        ///// </summary>
        //[TestMethod]
        //public void ConvertIfcRectangleHollowProfileToBRepTest()
        //{
          
        //    using (var m = IfcModelBuilder.CreateandInitModel())
        //    {
        //        using (var txn = m.BeginTransaction())
        //        {
        //            //add a shape
        //            //Create a Definition shape to hold the geometry
        //            var shape = m.Instances.New<IfcShapeRepresentation>();
        //            shape.ContextOfItems = m.IfcProject.ModelContext();
        //            shape.RepresentationType = "Brep";
        //            shape.RepresentationIdentifier = "Body";

        //            //Create a Product Definition and add the model geometry to the wall
        //            var rep = m.Instances.New<IfcProductDefinitionShape>();
        //            rep.Representations.Add(shape);
        //            var building = m.Instances.OfType<IfcBuilding>().FirstOrDefault();
        //            Assert.IsNotNull(building,"Failed to find Building");
        //            building.Representation = rep;

                    

        //            var block = IfcModelBuilder.MakeExtrudedAreaSolid(m, IfcModelBuilder.MakeRectangleHollowProfileDef(m,10,20, 1),30);
        //            var solid = _xbimGeometryCreator.CreateSolid(block);
                   
        //            var brep = _xbimGeometryCreator.CreateFacetedBrep(m, solid);

        //            shape.Items.Add(brep);

        //            var solid2 = _xbimGeometryCreator.CreateSolidSet(brep); //round trip it
                    
        //            txn.Commit();
        //            try
        //            {
        //                //Uncomment below to see the results in Ifc
        //                //m.SaveAs("brep.ifc", XbimStorageType.IFC);
        //            }
        //            catch (Exception)
        //            {

        //                Assert.IsTrue(false, "Failed to save the results to Ifc");
        //            }
        //            Assert.IsTrue(solid2.Count == 1, "Expected one solid");
        //            IfcCsgTests.GeneralTest(solid2.First);
        //            int volDiff = (int)Math.Abs(solid.Volume - solid2.First.Volume); //nearest cubic millimetre
        //            Assert.IsTrue(volDiff==0, "Volume of round tripped solid is not the same");

           
        //        }
        //    }
        //}

        ///// <summary>
        ///// Converts a RectangleHollowProfile in XbimSolid format to an IfcFacetedBrep ad back again 
        ///// </summary>
        //[TestMethod]
        //public void ConvertIfcCircleHollowProfileToBRepTest()
        //{

        //    using (var m = IfcModelBuilder.CreateandInitModel())
        //    {
        //        using (var txn = m.BeginTransaction())
        //        {
        //            //add a shape
        //            //Create a Definition shape to hold the geometry
        //            var shape = m.Instances.New<IfcShapeRepresentation>();
        //            shape.ContextOfItems = m.IfcProject.ModelContext();
        //            shape.RepresentationType = "Brep";
        //            shape.RepresentationIdentifier = "Body";

        //            //Create a Product Definition and add the model geometry to the wall
        //            var rep = m.Instances.New<IfcProductDefinitionShape>();
        //            rep.Representations.Add(shape);
        //            var building = m.Instances.OfType<IfcBuilding>().FirstOrDefault();
        //            Assert.IsNotNull(building, "Failed to find Building");
        //            building.Representation = rep;

                    

        //            var block = IfcModelBuilder.MakeExtrudedAreaSolid(m, IfcModelBuilder.MakeCircleHollowProfileDef(m, 1000, 500), 3000);
        //            var solid = _xbimGeometryCreator.CreateSolid(block);

        //            var brep = _xbimGeometryCreator.CreateFacetedBrep(m, solid);

        //            shape.Items.Add(brep);

        //            var solid2 = _xbimGeometryCreator.CreateSolidSet(brep); //round trip it
                   
        //            txn.Commit();
        //            try
        //            {
        //                //Uncomment below to see the results in Ifc
        //                //m.SaveAs("brep.ifc", XbimStorageType.IFC);
        //            }
        //            catch (Exception)
        //            {

        //                Assert.IsTrue(false, "Failed to save the results to Ifc");
        //            }
        //            Assert.IsTrue(solid2.Count == 1, "Expected one solid");
        //            IfcCsgTests.GeneralTest(solid2.First);
                    
        //            Assert.IsTrue(brep.Outer.CfsFaces.Count == solid2.First.Faces.Count, "Number of faces in round tripped solid is not the same");


        //        }
        //    }
        //}
    }
}
