
using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{

    public class BooleanFactoryTests
    {

        #region Setup



        private readonly MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        private readonly IXModelGeometryService _modelSvc;
        private readonly IXbimGeometryServicesFactory factory;
        private readonly ILoggerFactory _loggerFactory;

        const double Precision = 1e-5;
        const double PrecisionMax = 0.1;


        public BooleanFactoryTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            this.factory = factory;
            _loggerFactory = loggerFactory;
            _dummyModel.ModelFactors = new XbimModelFactors(angToRads: 1, 0.001, Precision);
            _modelSvc = factory.CreateModelGeometryService(_dummyModel, loggerFactory);
        }


        #endregion


        [Fact]
        public void Can_union_two_coincidental_blocks()
        {

            var booleanResult = IfcMoq.IfcBooleanResultMoq();
            var booleanFactory = _modelSvc.BooleanFactory;

            var shape = booleanFactory.Build(booleanResult);
            Assert.True(shape.ShapeType == XShapeType.Solid || shape.ShapeType == XShapeType.Compound);
            var solid = shape as IXSolid;
            if (shape is IXCompound compound)
            {
                Assert.True(compound.IsSolidsOnly && compound.Solids.Count() == 1);
                solid = compound.Solids.First();
            }
            solid.Should().NotBeNull();
            solid?.Shells.Should().HaveCount(1); //one shell
            solid?.Shells.First().Faces.Should().HaveCount(6); //6 faces
        }
        [Fact]
        public void Can_cut_two_coincidental_blocks()
        {

            var booleanResult = IfcMoq.IfcBooleanResultMoq(boolOp: Ifc4.Interfaces.IfcBooleanOperator.DIFFERENCE);
            var booleanFactory = _modelSvc.BooleanFactory;
            Assert.ThrowsAny<Exception>(() => booleanFactory.Build(booleanResult));
        }
        /// <summary>
        /// These tests create two blocks and vary their distance apart to either union to one block  if they
        /// are <=1mm apart, otherwise 2, this shows fuzz tolerance is working correctly, which is default 1mm
        /// </summary>
        /// <param name="dispX"></param>
        /// <param name="dispY"></param>
        /// <param name="dispZ"></param>
        /// <param name="singleSolid"></param>
        [Theory]
        [InlineData(-10 - (PrecisionMax * 1.01), 0, 0, false)] //x2 face  more than Minimum gap apart, should not be read as coincidental
        [InlineData(10 + (PrecisionMax * 1.01), 0, 0, false)] //x2 face  more than Minimum gap apart, should not be read as coincidental
        [InlineData(-10 - PrecisionMax, 0, 0)] //x2 face  less than or equal to Minimum gap apart, should be read as coincidental
        [InlineData(10.0 + PrecisionMax, 0, 0)] //x2 face  less than or equal to Minimum gap apart, should be read as coincidental
        [InlineData(0, 0, -30)] //z-2 face coincidental
        [InlineData(0, -20, 0)] //-y2 face coincidental
        [InlineData(-10, 0, 0)] //-x2 face coincidental
        [InlineData(0, 0, 30)] //z2 face coincidental
        [InlineData(0, 20, 0)] //y2 face coincidental
        [InlineData(10, 0, 0)] //x2 face coincidental
        // [InlineData(0, 0, 0)] //all faces connected
        public void Can_union_two_face_connected_blocks(double dispX, double dispY, double dispZ, bool singleSolid = true)
        {
            //by default these blocks are all lenX =10, lenY = 20, lenZ = 30
            var booleanResult = IfcMoq.IfcBooleanResultMoq(displacementX: dispX, displacementY: dispY, displacementZ: dispZ);
            var booleanFactory = _modelSvc.BooleanFactory;
            var shape = booleanFactory.Build(booleanResult);
            if (singleSolid)
            {
                Assert.True(shape.ShapeType == XShapeType.Solid || shape.ShapeType == XShapeType.Compound);
                var solid = shape as IXSolid;
                if (shape is IXCompound compound)
                {
                    //var def = compound.BRepDefinition();
                    Assert.True(compound.IsSolidsOnly && compound.Solids.Count() == 1);
                    solid = compound.Solids.First();

                }
                solid.Shells.Should().HaveCount(1); //one shell
                solid.Shells.First().Faces.Should().HaveCount(10); //6 faces
            }
            else //the blocks should not join
            {

                Assert.True(shape.ShapeType == XShapeType.Compound);
                var compound = shape as IXCompound;
                //var def = compound.BRepDefinition();
                Assert.True(compound.IsSolidsOnly);
                compound.Solids.Should().HaveCount(2);
            }
        }

        /// <summary>
        /// Cutting two identical shapes with different positions will always return a single solid
        /// NB that when the tolerance is set to one millimeter, the cut will perform but result shape will have verices moved by  0.5mm 
        /// OCC divides the tolerance between the two shapes and places the results in the centre, however, the surface don't move
        /// this means the bounding box is as you would expect but the vertices on the coincidenct face are moved 0.5 and have a tolerance of 1, so 
        /// that they still lie on the surface mathematically
        /// </summary>
        /// <param name="dispX"></param>
        /// <param name="dispY"></param>
        /// <param name="dispZ"></param>
        [Theory]
        [InlineData(10, -10 - (PrecisionMax * 1.01), false)] //x2 face  more than Minimum gap apart, should not be read as coincidental, so first shape is untouched
        [InlineData(10, 10 + (PrecisionMax * 1.01), false)] //x2 face  more than Minimum gap apart, should not be read as coincidental, so first shape is untouched
        [InlineData(10, -10 - PrecisionMax, true)] //x2 face  less than or equal to Minimum gap apart, should be read as coincidental, first shape smaller
        [InlineData(10, 10 + PrecisionMax, true)] //x2 face  less than or equal to Minimum gap apart, should be read as coincidental

        public void Can_cut_two_face_connected_blocks(double lenX, double dispX, bool intersects)
        {

            //by default these blocks are all lenX =10, lenY = 20, lenZ = 30
            var booleanResult = IfcMoq.IfcBooleanResultMoq(boolOp: Ifc4.Interfaces.IfcBooleanOperator.DIFFERENCE, lenX: lenX, displacementX: dispX);
            var booleanFactory = _modelSvc.BooleanFactory;

            var shape = booleanFactory.Build(booleanResult);

            Assert.True(shape.ShapeType == XShapeType.Solid);
            var solid = shape as IXSolid;
#if DEBUG
            var def = solid.BrepString();
#endif


            var box = solid.Bounds();
            if (intersects)
            {
                var growth = 2 * (Math.Abs(dispX) - lenX);
                box.LenX.Should().BeApproximately(lenX + growth, Precision);
            }
            else //should be the first parameter block
            {
                box.LenX.Should().BeApproximately(lenX, Precision);
            }
        }
        [Theory]
        [InlineData(10, 5, true)] //should result in a block of length x = 5
        [InlineData(10, -5, true)] //should result in a block of length x = 5
        [InlineData(10, 10 - (Precision * 0.9), false)] //technically a failure to intersect is  within tolerance  of MinumumGap
        [InlineData(10, -10 - (Precision * 0.9), false)] //technicially a failure to intersect as  within tolerance  of MinumumGap
        public void Can_intersect_two_blocks(double lenX, double dispX, bool intersects)
        {

            //by default these blocks are all lenX =10, lenY = 20, lenZ = 30
            var booleanResult = IfcMoq.IfcBooleanResultMoq(
            boolOp: Ifc4.Interfaces.IfcBooleanOperator.INTERSECTION,
            lenX: lenX,
            displacementX: dispX);

            var booleanFactory = _modelSvc.BooleanFactory;
            if (!intersects) //will always return an empty shape
            {

                Assert.ThrowsAny<Exception>(() => booleanFactory.Build(booleanResult));
            }
            else
            {
                var shape = booleanFactory.Build(booleanResult);
                Assert.True(shape.ShapeType == XShapeType.Solid);
                var solid = shape as IXSolid;
#if DEBUG
                var def = solid.BrepString();
#endif

                var box = solid.Bounds();
                box.LenX.Should().BeApproximately(lenX - Math.Abs(dispX), Precision);
            }
        }

        [Theory]
        [InlineData(10)]
        public void Can_build_nested_boolean_results(int depth)
        {

            var booleanResult = IfcMoq.IfcDeepBooleanResultMoq(depth: depth, displacement: 10);
            var booleanFactory = _modelSvc.BooleanFactory;
            var shape = booleanFactory.Build(booleanResult);
            shape.Should().NotBeNull();
        }

        [Fact]
        public void Can_Clip_With_HalfSpace()
        {
            using var model = MemoryModel.OpenRead("testfiles/BooleanClippingWithHalfSpace.ifc");
            var geomEngine = factory.CreateGeometryEngineV6(model, _loggerFactory);
            var booleanOp = model.Instances[1] as IIfcBooleanClippingResult;
            var shape = geomEngine.Build(booleanOp);
            shape.Should().NotBeNull();
            shape.Should().BeAssignableTo<IXSolid>();
            ((IXSolid)shape).Volume.Should().BeApproximately(125458771.93626986, Precision);
        }


        /// <summary>
        /// The polygonally bound half spaces are extremely small and leave holes that are less than 0.2mm
        /// Xbim ignores holes of this size as in building terms it is an extreme level of detail and most likely a draughting error
        /// </summary>
        [Fact]
        public void Can_build_boolean_clipping_result_with_halfspaces()
        {
            using var model = MemoryModel.OpenRead("testfiles/boolean_clipping_result_with_halfspace.ifc");
            var geomEngine = factory.CreateGeometryEngineV6(model, _loggerFactory);
            var booleanOp = model.Instances[1] as IIfcBooleanClippingResult;
            var shape = geomEngine.Create(booleanOp) as IXbimSolid;
            shape.Should().NotBeNull();
            shape.Volume.Should().BeApproximately(3.4498500000004735, Precision);
        }

#if !DEBUG
        //[Theory]
        //[InlineData(5)]
        //public async Task Can_build_boolean_results_faster_with_Async(int depth)
        //{
            
        //    var booleanResult = IfcMoq.IfcDeepBooleanResultMoq(depth: depth, displacement: 10);
        //    var booleanService = _modelSvc.BooleanFactory;
           
            
        //    var sw = new Stopwatch();
        //    sw.Start();
        //    var taskResults = new List<Task<IXShape>>(10);
        //    for (int i = 0; i < 10; i++)
        //    {
        //        taskResults.Add(booleanService.BuildAsync(booleanResult));
        //    }
        //    await Task.WhenAll(taskResults);
        //    var asyncTime = sw.ElapsedMilliseconds;
        //    sw.Restart();
        //    var results = new List<IXShape>(10);
        //    for (int i = 0; i < 10; i++)
        //    {
        //        results.Add(booleanService.Build(booleanResult));
        //    }
        //    sw.Stop();
        //    var noneAsyncTime = sw.ElapsedMilliseconds;
        //    asyncTime.Should().BeLessThanOrEqualTo(noneAsyncTime);
        //    Console.WriteLine($"Async time = {asyncTime}, Sync time = {noneAsyncTime}");


        //}
#endif

        [Fact]
        public void Can_build_boolean_result_with_small_solids()
        {
            using var model = MemoryModel.OpenRead("testfiles/boolean_result_with_small_solids.ifc");
            var geomEngine = factory.CreateGeometryEngineV6(model, _loggerFactory);
            var booleanOp = model.Instances[1] as IIfcBooleanClippingResult;
            var shape = geomEngine.Create(booleanOp) as IXbimSolid;
            shape.Should().NotBeNull();
            shape.Volume.Should().BeApproximately(1.832928042298613e-5, Precision);
        }
        [Fact]
        public void Can_build_boolean_result_with_bad_polygonal_half_space_bounds()
        {
            using var model = MemoryModel.OpenRead("testfiles/boolean_result_with_bad_polygonal_half_space_bounds.ifc");
            var geomEngine = factory.CreateGeometryEngineV6(model, _loggerFactory);
            var booleanOp = model.Instances[1] as IIfcBooleanClippingResult;
            var shape = geomEngine.Create(booleanOp) as IXbimSolid;
            shape.Should().NotBeNull();
            shape.Volume.Should().BeApproximately(0.91613407890247534, Precision);
        }


        [Theory]
        [InlineData(@"TestFiles\IFC4TestFiles\wffdmcc3-_Navis - Existing.ifc")]
        public void CanBuildIfcClippingBooleanResult(string filePath)
        {
            // Arrange
            using var model = MemoryModel.OpenRead(filePath);
            var booleanResult = model.Instances[206469] as IIfcBooleanClippingResult;
            var modelSvc = factory.CreateModelGeometryService(model, _loggerFactory);

            // Act
            var solid = modelSvc.BooleanFactory.Build(booleanResult);

            // Assert
            solid.Should().NotBeNull();
        }

    }
}