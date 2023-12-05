using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;
using Xunit;
using Xunit.Abstractions;

namespace Xbim.Geometry.Engine.Tests
{
    public class SampleModelTests
    {
        public SampleModelTests(ITestOutputHelper testOutputHelper, ILogger<SampleModelTests> logger, ILoggerFactory loggerFactory)
        {
            TestOutputHelper = testOutputHelper;
            Logger = logger;
            LoggerFactory = loggerFactory;
        }

        public ITestOutputHelper TestOutputHelper { get; }
        public ILogger<SampleModelTests> Logger { get; }
        public ILoggerFactory LoggerFactory { get; }

        [InlineData(@"C:\Users\AndyWard\Downloads\je dunn - atl office_architecture_r20_detached.ifczip")]
        [InlineData(@"C:\Users\AndyWard\Downloads\taper\je dunn - atl office_architecture_r20_detached-IFC4\rsiuf2ey-{3D}.ifc")]
        [Theory]
        public void Test(string filename)
        {

            using (var model = MemoryModel.OpenRead(filename))
            {
                var context = new Xbim3DModelContext(model, LoggerFactory, Abstractions.XGeometryEngineVersion.V6);
                context.CreateContext();


            }
        }
    }
}
