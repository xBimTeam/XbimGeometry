using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.NetCore.Tests
{
    

    public class StorageServiceTests
    {
        #region Setup
        //const string SkipOrNot = null; // Run all tests
        const string SkipOrNot = "BrepDocumentManager cannot be run in the unit test domain"; // Skip all tests

        private readonly IXBRepDocumentManager _brepDocumentManager;
        private readonly IXbimGeometryServicesFactory factory;
        static ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        static MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        private readonly IXModelGeometryService _modelSvc;
        public StorageServiceTests(IXBRepDocumentManager brepDocumentManager, IXbimGeometryServicesFactory factory)
        {
           
           _brepDocumentManager = brepDocumentManager;
            this.factory = factory;
            _modelSvc = factory.CreateModelGeometryService(_dummyModel, loggerFactory);
        }

        #endregion

       
        [Fact(Skip = SkipOrNot)]
        public void Can_Create_Coloured_Shapes_Document()
        {
          
            var materialFactory = _modelSvc.MaterialFactory;
            using var storageDoc = _brepDocumentManager.NewDocument();
            var visMaterial = materialFactory.BuildVisualMaterial("Purple");
            visMaterial.DiffuseColor = materialFactory.BuildColourRGB(1, 0, 1);
            visMaterial.Shininess = 0.3f;
            visMaterial.Transparency = 0.5f;

            var assembly = storageDoc.CreateAssembly("assembly");

            assembly.IsTopLevel.Should().BeTrue();
            assembly.IsSubShape.Should().BeFalse();
            var solidFactory = _modelSvc.SolidFactory;
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);
            var oneMeter = _modelSvc.OneMeter;
            _modelSvc.MeshFactors.LinearDefection = oneMeter * 0.004;
            block.Triangulate(_modelSvc.MeshFactors);
            assembly.AddComponent("Block", 20, block);
            assembly.VisualMaterial = visMaterial;
            _brepDocumentManager.SaveAs($"{nameof(Can_Create_Coloured_Shapes_Document)}.flex", storageDoc).Should().BeTrue();
        }


        [Fact(Skip = SkipOrNot)]
        public void Can_Add_Shapes_To_Document()
        {
           
            var compoundFactory =_modelSvc.CompoundFactory;
            var materialFactory = _modelSvc.MaterialFactory;
            using var storageDoc = _brepDocumentManager.NewDocument();
            //create a pair of nested assemblies
            var assembly = storageDoc.CreateAssembly("assembly");
            var subAssembly = assembly.AddAssembly("sub assembly");
            //create some shapes
            var solidFactory = _modelSvc.SolidFactory;
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);
            var cylinderMoq = IfcMoq.IfcRightCircularConeMoq();
            var cylinder = solidFactory.Build(cylinderMoq);

            //triangulate the shapes
            var oneMeter = _modelSvc.OneMeter;
            _modelSvc.MeshFactors.LinearDefection = oneMeter * 0.004;
            block.Triangulate(_modelSvc.MeshFactors);
            cylinder.Triangulate(_modelSvc.MeshFactors);

            //add them to the asemblies
            var storedBlock = subAssembly.AddShape("block", block);
            var storedCylinder = assembly.AddShape("cylinder", cylinder);

            assembly.AddComponent("cylinder2", 20, cylinder);
            subAssembly.AddComponent("block2", 30, block);

            var purple = materialFactory.BuildVisualMaterial("Purple");
            purple.DiffuseColor = materialFactory.BuildColourRGB(1, 0, 1);
            purple.Shininess = 0.3f;
            purple.Transparency = 0.5f;
            
            var purpleMat = storageDoc.AddVisualMaterial(purple);

            var green = materialFactory.BuildVisualMaterial("Green");
            green.DiffuseColor = materialFactory.BuildColourRGB(0, 1, 0);
            green.Shininess = 0.3f;
            green.Transparency = 0.5f;
            
            var greenMat = storageDoc.AddVisualMaterial(green);
            storedBlock.VisualMaterial = purpleMat;
            storedCylinder.VisualMaterial = greenMat;

            //update all entities
            storageDoc.UpdateAssemblies();

            _brepDocumentManager.SaveAs($"{nameof(Can_Add_Shapes_To_Document)}.flex", storageDoc).Should().BeTrue();
        }

        [Fact(Skip = SkipOrNot)]
        public void Can_Add_SubShapes_To_Document()
        {
           
            var materialFactory = _modelSvc.MaterialFactory;
            using var storageDoc = _brepDocumentManager.NewDocument();
            var assembly = storageDoc.CreateAssembly("assembly");

            var solidFactory = _modelSvc.SolidFactory;
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);

            var oneMeter = _modelSvc.OneMeter;
            _modelSvc.MeshFactors.LinearDefection = oneMeter * 0.004;
            block.Triangulate(_modelSvc.MeshFactors);
            var purple = materialFactory.BuildVisualMaterial("Purple");
            purple.DiffuseColor = materialFactory.BuildColourRGB(1, 0, 1);
            purple.Shininess = 0.3f;
            purple.Transparency = 0.5f;

            var purpleMat = storageDoc.AddVisualMaterial(purple);

            var green = materialFactory.BuildVisualMaterial("Green");
            green.DiffuseColor = materialFactory.BuildColourRGB(0, 1, 0);
            green.Shininess = 0.3f;
            green.Transparency = 0.5f;

            var greenMat = storageDoc.AddVisualMaterial(green);

            var storedBlock = assembly.AddShape("Main Block", block);
            var component = assembly.AddComponent("ref block", 10, block);
            //one face of the block as a subshape
            var storedBlockFace = storedBlock.AddSubShape("Sub Block", block.Shells.FirstOrDefault().Faces.FirstOrDefault(), true);
            //give it a different colour
            storedBlock.VisualMaterial = purpleMat;
            storedBlockFace.VisualMaterial = greenMat;
            storageDoc.UpdateAssemblies();
            _brepDocumentManager.SaveAs($"{nameof(Can_Add_SubShapes_To_Document)}.flex", storageDoc).Should().BeTrue();
        }

        [Fact(Skip = SkipOrNot)]
        public void Can_Add_MultiShape_Assembly_To_Document()
        {
            
            using var storageDoc = _brepDocumentManager.NewDocument();
            var assembly = storageDoc.CreateAssembly("assembly");

            var assembly2 = storageDoc.CreateAssembly("assembly");

            var solidFactory = _modelSvc.SolidFactory;
            var compoundFactory = _modelSvc.CompoundFactory;
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);
            var cylinderMoq = IfcMoq.IfcRightCircularConeMoq();
            var cylinder = solidFactory.Build(cylinderMoq);

            //Create a compound to hold all door parts
            var doorParts = compoundFactory.CreateEmpty();
            doorParts.Add(block);
            doorParts.Add(cylinder);
            //var doorPart1 = doorAssembly.AddShape("Block", block);
            //var doorPart2 = doorAssembly.AddShape("Cylinder", cylinder);
            assembly.AddComponent("parts", 10, doorParts);
            _brepDocumentManager.SaveAs($"{nameof(Can_Add_MultiShape_Assembly_To_Document)}.xbf", storageDoc).Should().BeTrue();
        }
        [Fact(Skip = SkipOrNot)]
        public void Can_Create_And_Retrieve_Materials_By_Label()
        {
           
            var materialFactory = _modelSvc.MaterialFactory;
            using var storageDoc = _brepDocumentManager.NewDocument();

            var purple = materialFactory.BuildVisualMaterial("purple");

            storageDoc.AddVisualMaterial(purple);

            var red = materialFactory.BuildVisualMaterial("red");
            storageDoc.AddVisualMaterial(red);
            var green = materialFactory.BuildVisualMaterial("green");
            storageDoc.AddVisualMaterial(green);
            var materials = storageDoc.GetMaterials();
            materials.Count().Should().Be(3);
            _brepDocumentManager.SaveAs($"{nameof(Can_Create_And_Retrieve_Materials_By_Label)}.xbf", storageDoc).Should().BeTrue();

        }

        [Fact(Skip = SkipOrNot)]
        public void Can_Set_Physical_Based_Render()
        {
           
            var materialFactory = _modelSvc.MaterialFactory;
            var solidFactory = _modelSvc.SolidFactory;
            using var storageDoc = _brepDocumentManager.NewDocument();

            var purple = materialFactory.BuildVisualMaterial("Purple");
            purple.DiffuseColor = materialFactory.BuildColourRGB(1, 0, 1);
            purple.Shininess = 0.3f;
            purple.Transparency = 0.5f;
            var purpleMat = storageDoc.AddVisualMaterial(purple);
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);
            var shapeLabel = storageDoc.CreateAssembly("Root");
            var blocklabel = shapeLabel.AddComponent("block", 10, block);
            storageDoc.SetMaterial(block, purpleMat);

            _brepDocumentManager.SaveAs($"{nameof(Can_Set_Physical_Based_Render)}.xbf", storageDoc).Should().BeTrue();
        }
        [Fact(Skip = SkipOrNot)]
        public void Can_Remove_Shape()
        {
            using var storageDoc = _brepDocumentManager.NewDocument();
            var shapeLabel = storageDoc.CreateAssembly("Root");
            storageDoc.Shapes.Should().NotBeEmpty();
            storageDoc.RemoveAssembly(shapeLabel).Should().BeTrue();
            storageDoc.Shapes.Should().BeEmpty();
        }


        [Fact(Skip = SkipOrNot)]
        public void Can_Create_And_Retrieve_Materials_By_Shape()
        {
            
           
            var solidFactory = _modelSvc.SolidFactory;
            var materialFactory = _modelSvc.MaterialFactory;
            using var storageDoc = _brepDocumentManager.NewDocument();
            
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);
            var shapeLabel = storageDoc.CreateAssembly("Root");
            var blocklabel = shapeLabel.AddComponent("block", 10, block);
            var purple = materialFactory.BuildVisualMaterial("purple");
            purple = storageDoc.AddVisualMaterial(purple);


            storageDoc.SetMaterial(block, purple);
            var retrievedLabel = storageDoc.GetMaterial(block);
            retrievedLabel.Should().NotBeNull();
            retrievedLabel.Name.Should().Be("purple");
            storageDoc.UpdateAssemblies();
            _brepDocumentManager.SaveAs($"{nameof(Can_Create_And_Retrieve_Materials_By_Shape)}.xbf", storageDoc).Should().BeTrue();
        }

        [Fact(Skip = SkipOrNot)]
        public void Can_Retrieve_Shape_Material()
        {
            
            
            var materialFactory = _modelSvc.MaterialFactory;
            var solidFactory = _modelSvc.SolidFactory;
            using var storageDoc = _brepDocumentManager.NewDocument();
            var purple = materialFactory.BuildVisualMaterial("Purple");
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);
            var shapeLabel = storageDoc.CreateAssembly("Root");
            var blocklabel = shapeLabel.AddComponent("block", 1, block);
            var purpleMat = storageDoc.AddVisualMaterial(purple);
            blocklabel.VisualMaterial = purpleMat;
            var retrievedLabel = storageDoc.GetMaterial(blocklabel);
            retrievedLabel.Name.Should().Be("Purple");
        }



        [Fact(Skip = SkipOrNot)]
        public void Can_Store_Volume_On_Assembly()
        {
            
            using var storageDoc = _brepDocumentManager.NewDocument();
            var solidFactory = _modelSvc.SolidFactory;
            var products = storageDoc.CreateAssembly("Products");
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);
            var blockLabel = products.AddComponent("Block", 10, block);
            blockLabel.Volume = block.Volume;
            blockLabel.Volume.Should().Be(block.Volume);
            storageDoc.UpdateAssemblies();
            _brepDocumentManager.SaveAs($"{nameof(Can_Store_Volume_On_Assembly)}.xbf", storageDoc).Should().BeTrue();
        }

       
       

        [Fact(Skip = SkipOrNot)]
        public async Task Can_Create_Multiple_Assemblies_ConcurrentlyAsync()
        {
           
            var tasks = new List<Task>();
            for (int i = 0; i < 20; i++)
            {
                tasks.Add(Task.Run(() =>
                {
                    

                    //set up a storage document 
                    using var storageDoc = _brepDocumentManager.NewDocument();
                    var products = storageDoc.CreateAssembly("Product");
                    var openings = storageDoc.CreateAssembly("Openings");
                    var projections = storageDoc.CreateAssembly("Projections");
                    
                    var solidFactory = _modelSvc.SolidFactory;

                    var blockMoq = IfcMoq.IfcBlockMoq();
                    var block = solidFactory.Build(blockMoq);
                    openings.AddComponent("Opening 1", 10, block);
                    var cylinderMoq = IfcMoq.IfcRightCircularConeMoq();
                    var cylinder = solidFactory.Build(cylinderMoq);
                    projections.AddComponent("Cylinder", 10, cylinder);
                    storageDoc.UpdateAssemblies();

                    storageDoc.Shapes.Where(s => s.Name == "Openings").Count().Should().Be(1);
                    openings.Components.Count().Should().Be(1);
                    products.Components.Count().Should().Be(0);
                    openings.Components.First().Shape.Should().NotBeNull();
                    openings.Components.First().Shape.Should().BeAssignableTo<IXSolid>();
                    openings.AddComponent("Opening 2", 10, cylinder);
                    _brepDocumentManager.SaveAs($"MultiAssembly{i}.xbf", storageDoc).Should().BeTrue();
                }
                ));
                await Task.WhenAll(tasks);
            }
        }

       
        
    }

}

