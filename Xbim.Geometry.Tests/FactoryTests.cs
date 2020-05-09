using Microsoft.Extensions.Logging;
using System;
using Xbim.Geometry.Engine.Interop.Factories;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Tests
{
    public class FactoryTests
    {
        //static ILoggerFactory loggerFactory;
        //static FactoryTests()
        //{
        //    loggerFactory = LoggerFactory.Create(builder =>
        //    {
        //        builder
        //            .AddFilter("Xbim", LogLevel.Trace)
        //            .AddConsole();
        //    });
        //}
        [Fact]
        public void Can_build_vertex_factory()
        {
            Console.WriteLine("sasasa");
            using var loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
            //using (var memoryModel = new MemoryModel(new EntityFactoryIfc4()))
            //{

            //    var vertexFactory = FactoryBuilder.BuildVertexFactory(memoryModel, loggerFactory);
            //}
        }
    }
}
