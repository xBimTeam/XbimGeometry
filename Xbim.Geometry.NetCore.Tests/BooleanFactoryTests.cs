using Extensions.Logging.ListOfString;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Factories;
using Xbim.Geometry.Services;
using Xbim.Ifc4;
using Xbim.IO.Memory;

namespace Xbim.Geometry.NetCore.Tests
{
    [TestClass]
    public class BooleanFactoryTests
    {

        #region Class Setup
        static IHost serviceHost;
        static IModel model;
        static IServiceScope _modelScope;
        [ClassInitialize]
        static public async System.Threading.Tasks.Task InitialiseAsync(TestContext context)
        {
            model = new MemoryModel(new EntityFactoryIfc4());
            serviceHost = CreateHostBuilder().Build();
            await serviceHost.StartAsync();
            _modelScope = serviceHost.Services.CreateScope();

        }
        public static IHostBuilder CreateHostBuilder() =>
        Host.CreateDefaultBuilder()
            .ConfigureServices((hostContext, services) =>
            {
                services.AddHostedService<GeometryServicesHost>()
                .AddSingleton<IXLoggingService, LoggingService>()
                .AddSingleton<IXShapeService, ShapeService>()
                .AddScoped<IXBooleanService, BooleanFactory>()
                .AddScoped<IXModelService, ModelService>(sp =>
                    new ModelService(IfcMoq.IfcModelMock(millimetre: 1, precision: 1e-5, radianFactor: 1), minGapSize: 1.0));
            })
        .ConfigureLogging((hostContext, loggingBuilder) =>
        {
            loggingBuilder.AddProvider(new StringListLoggerProvider(new StringListLogger(new List<string>(), name: "LoggingService")));
        });



        [ClassCleanup]
        static public async System.Threading.Tasks.Task CleanupAsync()
        {
            model = null;
            await serviceHost.StopAsync();
        }
        #endregion

        [TestMethod]
        public void Can_union_two_coincidental_blocks()
        {
            var booleanResult = IfcMoq.IfcBooleanResultMoq();
            var booleanFactory = _modelScope.ServiceProvider.GetRequiredService<IXBooleanService>();

            var shape = booleanFactory.Build(booleanResult);
            Assert.IsTrue(shape.ShapeType == XShapeType.Solid || shape.ShapeType == XShapeType.Compound);
            var compound = shape as IXCompound;
            var solid = shape as IXSolid;
            if (compound != null)
            {
                Assert.IsTrue(compound.IsSolidsOnly && compound.Solids.Count() == 1);
                solid = compound.Solids.First();
            }
            Assert.AreEqual(1, solid.Shells.Count()); //one shell
            Assert.AreEqual(6, solid.Shells.First().Faces.Count()); //6 faces
            //var def = solid.BRepDefinition();

        }
        /// <summary>
        /// These tests create two blocks and vary their distance apart to either union to one block  if they
        /// are <=1mm apart, otherwise 2, this shows fuzz tolerance is working correctly, which is default 1mm
        /// </summary>
        /// <param name="dispX"></param>
        /// <param name="dispY"></param>
        /// <param name="dispZ"></param>
        /// <param name="singleSolid"></param>
        [DataTestMethod]

        [DataRow(-11.1, 0, 0, false)] //x2 face  more than 1 millimeter apart, should not be read as coincidental
        [DataRow(11.5, 0, 0, false)] //x2 face  more than 1 millimeter apart, should not be read as coincidental
        [DataRow(-10.99999, 0, 0)] //x2 face  less than 1 millimeter apart, should be read as coincidental
        [DataRow(11.0, 0, 0)] //x2 face  less than 1 millimeter apart, should be read as coincidental
        [DataRow(0, 0, -30)] //z-2 face coincidental
        [DataRow(0, -20, 0)] //-y2 face coincidental
        [DataRow(-10, 0, 0)] //-x2 face coincidental
        [DataRow(0, 0, 30)] //z2 face coincidental
        [DataRow(0, 20, 0)] //y2 face coincidental
        [DataRow(10, 0, 0)] //x2 face coincidental
        [DataRow(0, 0, 0)] //all faces connected
        public void Can_union_two_face_connected_blocks(double dispX, double dispY, double dispZ, bool singleSolid = true)
        {
            //by default these blocks are all lenX =10, lenY = 20, lenZ = 30
            var booleanResult = IfcMoq.IfcBooleanResultMoq(displacementX: dispX, displacementY: dispY, displacementZ: dispZ);
            var booleanFactory = _modelScope.ServiceProvider.GetRequiredService<IXBooleanService>();
            var shape = booleanFactory.Build(booleanResult);
            if (singleSolid)
            {
                Assert.IsTrue(shape.ShapeType == XShapeType.Solid || shape.ShapeType == XShapeType.Compound);
                var compound = shape as IXCompound;
                var solid = shape as IXSolid;
                if (compound != null)
                {
                    //var def = compound.BRepDefinition();
                    Assert.IsTrue(compound.IsSolidsOnly && compound.Solids.Count() == 1);
                    solid = compound.Solids.First();

                }
                Assert.AreEqual(1, solid.Shells.Count()); //one shell
                Assert.AreEqual(6, solid.Shells.First().Faces.Count()); //6 faces
            }
            else //the blocks should not join
            {

                Assert.IsTrue(shape.ShapeType == XShapeType.Compound);
                var compound = shape as IXCompound;
                //var def = compound.BRepDefinition();
                Assert.IsTrue(compound.IsSolidsOnly);
                Assert.AreEqual(2, compound.Solids.Count());
            }
        }
    }
}
