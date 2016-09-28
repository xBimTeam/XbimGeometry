namespace Ifc4GeometryTests
{
#if USE_CARVE_CSG

    [TestClass]
    [DeploymentItem(@"SolidTestFiles\")]
    public class XbimFacetedMeshTests
    {
        [TestMethod]
        public void CreateXbimFacetedSolidFromIfcBlock()
        {

            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {
                    
                    var ifcBlock = IfcModelBuilder.MakeBlock(m, 1000, 2000, 3000);
                    var solid = XbimGeometryCreator.CreateSolid(ifcBlock);
                    var faceted = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    Assert.IsTrue(faceted.Faces.First.OuterBound.Edges.Count == 4, "Should have four edges");
                    Assert.IsTrue(faceted.Vertices.Count == 8, "there should be 8 points in a block");
                    Assert.IsTrue(Math.Abs(faceted.BoundingBox.SizeX - 1000) <= m.ModelFactors.Precision, "Error in X value");
                    Assert.IsTrue(Math.Abs(faceted.BoundingBox.SizeY - 2000) <= m.ModelFactors.Precision, "Error in Y value");
                    Assert.IsTrue(Math.Abs(faceted.BoundingBox.SizeZ - 3000) <= m.ModelFactors.Precision, "Error in Z value");
                    Assert.IsTrue(Math.Abs(solid.Volume - faceted.Volume) <= m.ModelFactors.Precision, "Error in volume calc");
                }
            }
        }

        [TestMethod]
        public void CreateXbimFacetedSolidFromIfcRightCircularCylinder()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 1, 4); //1 metre by 4 metre
                    var solid = XbimGeometryCreator.CreateSolid(cylinder);
                    var faceted = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);

                }
            }
        }
        [TestMethod]
        public void CreateXbimFacetedSolidFromIfcRectangleHollowProfileDef()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 10, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = XbimGeometryCreator.CreateSolid(extrude);
                    var faceted = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    foreach (var face in faceted.Faces)
                    {
                        Assert.IsTrue(face.OuterBound.Edges.Count == 4, "Each face should have four outer edges");
                    }
                    int hasInnerCount = 0;
                    foreach (var face in faceted.Faces)
                    {
                        if (face.InnerBounds.Count == 1)
                        {
                            hasInnerCount++;
                            Assert.IsTrue(face.InnerBounds.First.Edges.Count == 4, "Each face should have four inner edges");
                        }
                    }
                    Assert.IsTrue(hasInnerCount == 2, "There should be two faces with an innerbound");
                    Assert.IsTrue(faceted.Shells.Count == 1, "Should only be one shell");
                    Assert.IsTrue(faceted.Vertices.Count == faceted.Shells.First.Vertices.Count, "Should be one shell with the same number of vertices");
                }
            }
        }

        [TestMethod]
        public void ConvertXbimSolidFromXbimFacetedSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 10, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = XbimGeometryCreator.CreateSolid(extrude);
                    var faceted = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var solid2 = XbimGeometryCreator.CreateSolid(faceted);
                    Assert.IsTrue(solid.Shells.Count == solid2.Shells.Count, "Shell count differs");
                    Assert.IsTrue(solid.Faces.Count == solid2.Faces.Count, "Face count differs");
                    Assert.IsTrue(solid.Edges.Count == solid2.Edges.Count, "Edge count differs");
                    Assert.IsTrue(solid.Vertices.Count == solid2.Vertices.Count, "Vertex count differs");
                    Assert.IsTrue(Math.Abs(solid.Volume - solid2.Volume) < 0.001, "Volume differs");
                }
            }
        }

        [TestMethod]
        public void ConvertXbimSolidFromXbimFacetedSolidCurvedTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeCircleHollowProfileDef(m, 20, 2);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = XbimGeometryCreator.CreateSolid(extrude);
                    var faceted = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance * 10, 1);
                    var solid2 = XbimGeometryCreator.CreateSolid(faceted);
                    Assert.IsTrue(solid2.Shells.Count == faceted.Shells.Count, "Shell count differs");
                    Assert.IsTrue(solid2.Faces.Count == faceted.Faces.Count, "Face count differs");
                    Assert.IsTrue(solid2.Edges.Count == faceted.Edges.Count, "Edge count differs");
                    Assert.IsTrue(solid2.Vertices.Count == faceted.Vertices.Count, "Vertex count differs");
                    Assert.IsTrue(Math.Abs(faceted.Volume - solid2.Volume) < 0.001, "Volume differs");
                    Assert.IsTrue(Math.Abs(solid.Volume - solid2.Volume) < 30, "Volume differs");
                }
            }
        }

        [TestMethod]
        public void CutFacetedSolidFromFacetedSolid()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    IfcBlock ifcBlock1 = IfcModelBuilder.MakeBlock(m, 1000, 2000, 3000);
                    IfcBlock ifcBlock2 = IfcModelBuilder.MakeBlock(m, 200, 200, 3000);
                    IfcBlock ifcBlock3 = IfcModelBuilder.MakeBlock(m, 200, 200, 3000);
                    ifcBlock2.Position.Location.X += 200;
                    ifcBlock2.Position.Location.Y += 200;
                    ifcBlock3.Position.Location.X += 200;
                    ifcBlock3.Position.Location.Y += 600;
                    var solid1 = XbimGeometryCreator.CreateSolid(ifcBlock1);
                    var solid2 = XbimGeometryCreator.CreateSolid(ifcBlock2);
                    var faceted1 = XbimGeometryCreator.CreateFacetedSolid(XbimGeometryCreator.CreateSolid(ifcBlock3), m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    var geom1 = solid1.Cut(solid2, m.ModelFactors.Precision);
                    var solidSet = (IXbimSolidSet)geom1;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    var faceted2 = XbimGeometryCreator.CreateFacetedSolid(solidSet.First, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    var geom2 = faceted2.Cut(faceted1, m.ModelFactors.Precision);
                    solidSet = (IXbimSolidSet)geom2;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    var faceted3 = solidSet.First;

                    var solid6 = XbimGeometryCreator.CreateSolid(faceted3);

                    Assert.IsTrue(Math.Abs(solid1.BoundingBox.SizeX - solid6.BoundingBox.SizeX) < m.ModelFactors.Precision * 3, "BoundingBox size X error");
                    Assert.IsTrue(Math.Abs(solid1.BoundingBox.SizeY - solid6.BoundingBox.SizeY) < m.ModelFactors.Precision * 3, "BoundingBox size Y error");
                    Assert.IsTrue(Math.Abs(solid1.BoundingBox.SizeZ - solid6.BoundingBox.SizeZ) < m.ModelFactors.Precision * 3, "BoundingBox size Z error");
                    Assert.IsTrue(Math.Abs(solid6.Volume - faceted3.Volume) <= m.ModelFactors.Precision, "Error in volume calc");
                    Assert.IsTrue(Math.Abs(solid6.Volume - faceted3.Volume) <= m.ModelFactors.Precision, "Error in volume calc");
                }
            }
        }

        [TestMethod]
        public void UnionFacetedSolidFromFacetedSolid()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var ifcBlock1 = IfcModelBuilder.MakeBlock(m, 1000, 2000, 3000);
                    var ifcBlock2 = IfcModelBuilder.MakeBlock(m, 200, 200, 3000);
                    var ifcBlock3 = IfcModelBuilder.MakeBlock(m, 200, 200, 3000);
                    ifcBlock2.Position.Location.X -= 180;
                    ifcBlock2.Position.Location.Y += 200;
                    ifcBlock3.Position.Location.X -= 180;
                    ifcBlock3.Position.Location.Y += 600;
                    var solid1 = XbimGeometryCreator.CreateSolid(ifcBlock1);
                    var solid2 = XbimGeometryCreator.CreateSolid(ifcBlock2);
                    var faceted1 = XbimGeometryCreator.CreateFacetedSolid(XbimGeometryCreator.CreateSolid(ifcBlock3), m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    var geom1 = solid1.Union(solid2, m.ModelFactors.Precision);
                    var solidSet = (IXbimSolidSet)geom1;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    var faceted2 = XbimGeometryCreator.CreateFacetedSolid(solidSet.First, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    var geom2 = faceted2.Union(faceted1, m.ModelFactors.Precision);
                    solidSet = (IXbimSolidSet)geom2;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    var faceted3 = solidSet.First;

                    var solid6 = XbimGeometryCreator.CreateSolid(faceted3);

                    Assert.IsTrue(Math.Abs(solid1.BoundingBox.SizeX + 180 - solid6.BoundingBox.SizeX) < m.ModelFactors.Precision * 3, "BoundingBox size X error");
                    Assert.IsTrue(Math.Abs(solid1.BoundingBox.SizeY - solid6.BoundingBox.SizeY) < m.ModelFactors.Precision * 3, "BoundingBox size Y error");
                    Assert.IsTrue(Math.Abs(solid1.BoundingBox.SizeZ - solid6.BoundingBox.SizeZ) < m.ModelFactors.Precision * 3, "BoundingBox size Z error");
                    Assert.IsTrue(Math.Abs(solid6.Volume - faceted3.Volume) <= m.ModelFactors.Precision, "Error in volume calc");
                    Assert.IsTrue(Math.Abs(solid6.Volume - faceted3.Volume) <= m.ModelFactors.Precision, "Error in volume calc");
                }
            }
        }


        [TestMethod]
        public void IntersectFacetedSolidFromFacetedSolid()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var ifcBlock1 = IfcModelBuilder.MakeBlock(m, 1000, 2000, 3000);
                    var ifcBlock2 = IfcModelBuilder.MakeBlock(m, 200, 200, 3000);
                    var ifcBlock3 = IfcModelBuilder.MakeBlock(m, 200, 200, 3000);
                    ifcBlock2.Position.Location.X -= 180;
                    ifcBlock2.Position.Location.Y += 200;
                    ifcBlock3.Position.Location.X -= 180;
                    ifcBlock3.Position.Location.Y += 600;
                    var solid1 = XbimGeometryCreator.CreateSolid(ifcBlock1);
                    var solid2 = XbimGeometryCreator.CreateSolid(ifcBlock2);
                    var faceted1 = XbimGeometryCreator.CreateFacetedSolid(XbimGeometryCreator.CreateSolid(ifcBlock3), m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    var geom1 = solid1.Union(solid2, m.ModelFactors.Precision);
                    var solidSet = (IXbimSolidSet)geom1;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    var faceted2 = XbimGeometryCreator.CreateFacetedSolid(solidSet.First, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    var geom2 = faceted2.Intersection(faceted1, m.ModelFactors.Precision);
                    solidSet = (IXbimSolidSet)geom2;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    var faceted3 = solidSet.First;

                    var solid6 = XbimGeometryCreator.CreateSolid(faceted3);
                    //var w = new XbimOccWriter();
                    //w.Write(solid6, "d:\\xbim\\s");
                    Assert.IsTrue(Math.Abs(solid6.BoundingBox.SizeX - 20) < m.ModelFactors.Precision * 3, "BoundingBox size X error");
                    Assert.IsTrue(Math.Abs(solid6.BoundingBox.SizeY - 200) < m.ModelFactors.Precision * 3, "BoundingBox size Y error");
                    Assert.IsTrue(Math.Abs(solid6.BoundingBox.SizeZ - 3000) < m.ModelFactors.Precision * 3, "BoundingBox size Z error");
                    Assert.IsTrue(Math.Abs(solid6.Volume - 20 * 3000 * 200) <= m.ModelFactors.Precision, "Error in volume calc");

                }
            }
        }

        [TestMethod]
        public void BooleanPerformanceTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var ifcBlock1 = IfcModelBuilder.MakeBlock(m, 1000, 2000, 3000);
                    var ifcBlock2 = IfcModelBuilder.MakeBlock(m, 200, 200, 3000);
                    //var ifcBlock1 = IfcModelBuilder.MakeRightCircularCylinder(m, 2000, 3000);
                    //var ifcBlock2 = IfcModelBuilder.MakeRightCircularCylinder(m, 2000, 3000); 
                    //var ifcBlock1 = IfcModelBuilder.MakeExtrudedAreaSolid(m, IfcModelBuilder.MakeRectangleHollowProfileDef(m, 2000, 3000, 500), 4000);
                    //var ifcBlock2 = IfcModelBuilder.MakeExtrudedAreaSolid(m, IfcModelBuilder.MakeRectangleHollowProfileDef(m, 2000, 3000, 500), 4000); 

                    ifcBlock2.Position.Location.X -= 180;
                    ifcBlock2.Position.Location.Y += 200;

                    var solid1 = XbimGeometryCreator.CreateSolid(ifcBlock1);
                    var solid2 = XbimGeometryCreator.CreateSolid(ifcBlock2);
                    var faceted1 = XbimGeometryCreator.CreateFacetedSolid(XbimGeometryCreator.CreateSolid(ifcBlock1), m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, 1);
                    var faceted2 = XbimGeometryCreator.CreateFacetedSolid(XbimGeometryCreator.CreateSolid(ifcBlock2), m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, 1);
                    var solid3 = solid1.Cut(solid2, m.ModelFactors.Precision);
                    solid3 = faceted1.Cut(faceted2, m.ModelFactors.Precision);

                    var sw = new Stopwatch();
                    sw.Start();
                    for (int i = 0; i < 10; i++)
                    {
                        solid3 = solid1.Cut(solid2, m.ModelFactors.Precision);
                    }
                    double time1 = sw.ElapsedMilliseconds;
                    sw.Restart();
                    for (int i = 0; i < 10; i++)
                    {
                        solid3 = faceted1.Cut(faceted2, m.ModelFactors.Precision);
                    }
                    double time2 = sw.ElapsedMilliseconds;
                    Assert.IsTrue(time2 < time1, "Performance error");
                }
            }
        }


        [TestMethod]
        public void SectionFacetedSolidFromBlockTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block = IfcModelBuilder.MakeBlock(m, 10, 15, 20);


                    var solid = XbimGeometryCreator.CreateSolid(block);
                    var faceted1 = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    IfcPlane plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(block.Position.Location.X + 5, block.Position.Location.Y, block.Position.Location.Z), new XbimVector3D(-1, 0, 0), new XbimVector3D(0, 1, 0));
                    var cutPlane = XbimGeometryCreator.CreateFace(plane);

                    var section = faceted1.Section(cutPlane, m.ModelFactors.PrecisionBoolean);
                    if (section.First == null)
                    {
                        Assert.IsTrue(section.First != null, "Result should be a single face");
                        Assert.IsTrue(section.First.OuterBound.Edges.Count == 4, "4 edges are required of a section of a block");
                    }
                }
            }
        }

        [TestMethod]
        public void SectionFacetedSolidFromCylinderTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var solid = XbimGeometryCreator.CreateSolid(cylinder);
                    var faceted1 = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);

                    IfcPlane plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(cylinder.Position.Location.X + 1, cylinder.Position.Location.Y, cylinder.Position.Location.Z), new XbimVector3D(0, -1, 0), new XbimVector3D(1, 0, 0));
                    var cutPlane = XbimGeometryCreator.CreateFace(plane);
                    var section = faceted1.Section(cutPlane, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(section.First != null, "Result should be a face");
                    Assert.IsTrue(section.First.OuterBound.Edges.Count == 4, "4 edges are required for this section of a cylinder");
                    //repeat with section through cylinder
                    plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(cylinder.Position.Location.X, cylinder.Position.Location.Y, cylinder.Position.Location.Z + 1), new XbimVector3D(0, 0, 1), new XbimVector3D(1, 0, 0));
                    cutPlane = XbimGeometryCreator.CreateFace(plane);
                    section = faceted1.Section(cutPlane, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(section.First != null, "Result should be a face");
                    Assert.IsTrue(Math.Abs(section.First.Area - Math.PI * 20 * 20) < 5, "Area of cylinder seems incorrect");
                    Assert.IsTrue(section.First.InnerBounds.Count == 0, "0 inner wires are required for this section of a cylinder");
                }
            }
        }

        [TestMethod]
        public void ConvertSolidToTriangulatedFacetedSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 10, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = XbimGeometryCreator.CreateSolid(extrude);
                    var triangulated = XbimGeometryCreator.CreateTriangulatedSolid(solid, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);

                    Assert.IsTrue(solid.Shells.Count == triangulated.Shells.Count, "Shell count differs");
                    Assert.IsTrue(triangulated.Faces.Count == 32, "Face count differs");
                    Assert.IsTrue(solid.Edges.Count * 2 == triangulated.Edges.Count, "Edge count differs");
                    Assert.IsTrue(solid.Vertices.Count == triangulated.Vertices.Count, "Vertex count differs");
                    Assert.IsTrue(Math.Abs(solid.Volume - triangulated.Volume) < 0.001, "Volume differs");
                }
            }
        }

        [TestMethod]
        public void ReadWriteTriangulationOfPlanarSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 10, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = XbimGeometryCreator.CreateSolid(extrude);
                    TextWriter tw = new StringWriter();
                    XbimGeometryCreator.WriteTriangulation(tw, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, 0.5);
                    TextReader tr = new StringReader(tw.ToString());
                    IXbimGeometryObject geom = XbimGeometryCreator.ReadTriangulation(tr);
                    var triangulatedSold = geom as IXbimSolid;
                    Assert.IsNotNull(triangulatedSold, "Invalid solid returned");
                    Assert.IsTrue(Math.Abs(solid.Volume - triangulatedSold.Volume) < 0.001, "Volume differs");
                }
            }
        }

        [TestMethod]
        public void ReadWriteTriangulationOfNonPlanarSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeCircleHollowProfileDef(m, 20, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = XbimGeometryCreator.CreateSolid(extrude);
                    TextWriter tw = new StringWriter();
                    XbimGeometryCreator.WriteTriangulation(tw, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, 0.5);
                    TextReader tr = new StringReader(tw.ToString());
                    IXbimGeometryObject geom = XbimGeometryCreator.ReadTriangulation(tr);
                    var triangulatedSold = geom as IXbimSolid;
                    Assert.IsNotNull(triangulatedSold, "Invalid solid returned");
                    Assert.IsTrue(Math.Abs(solid.Volume - triangulatedSold.Volume) < 5, "Volume differs too much");
                }
            }
        }

        [TestMethod]
        public void ReadWriteTriangulationOfPlanarFacetedSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 10, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = XbimGeometryCreator.CreateSolid(extrude);
                    solid = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, 0.5);
                    TextWriter tw = new StringWriter();
                    XbimGeometryCreator.WriteTriangulation(tw, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, 0.5);
                    TextReader tr = new StringReader(tw.ToString());
                    IXbimGeometryObject geom = XbimGeometryCreator.ReadTriangulation(tr);
                    var triangulatedSold = geom as IXbimSolid;
                    Assert.IsNotNull(triangulatedSold, "Invalid solid returned");
                    Assert.IsTrue(Math.Abs(solid.Volume - triangulatedSold.Volume) < 0.001, "Volume differs");
                }
            }
        }

        [TestMethod]
        public void ReadWriteTriangulationOfNonPlanarFacetedSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var profile = IfcModelBuilder.MakeCircleHollowProfileDef(m, 20, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = XbimGeometryCreator.CreateSolid(extrude);
                    solid = XbimGeometryCreator.CreateFacetedSolid(solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, 0.5);
                    TextWriter tw = new StringWriter();
                    XbimGeometryCreator.WriteTriangulation(tw, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, 0.5);
                    TextReader tr = new StringReader(tw.ToString());
                    IXbimGeometryObject geom = XbimGeometryCreator.ReadTriangulation(tr);
                    var triangulatedSold = geom as IXbimSolid;
                    Assert.IsNotNull(triangulatedSold, "Invalid solid returned");
                    Assert.IsTrue(Math.Abs(solid.Volume - triangulatedSold.Volume) < 5, "Volume differs too much");
                }
            }
        }
    } 
#endif
}
