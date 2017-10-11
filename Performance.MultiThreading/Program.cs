using Google.Protobuf;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Geometry.Engine.Interop;
using Xbim.Geometry.Engine.Interop.Tests;
using Xbim.Geometry.ProtoBuffer;
using Xbim.Ifc4.Interfaces;

namespace Performance.MultiThreading
{
    class Program
    {
        static private IXbimGeometryEngine geomEngine;
        static private ILoggerFactory loggerFactory;
        static private ILogger logger;
        static Program()
        {
            loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<Program>();
        }
        static void Main(string[] args)
        {

            var path = Path.GetFullPath(@"..\..\..\Xbim.Geometry.Engine.Interop.Tests\TestFiles\CompositeProfileWithCutsTimeoutsTest.proto");
            using (var input = File.Open(path, FileMode.Open))
            {
                var shapeGeometryDtoCopy = new ShapeGeometryDTO();
                shapeGeometryDtoCopy.MergeDelimitedFrom(input);
                var time =  HelperFunctions.ConvertGeometryAllCompositesAtOnce(geomEngine, shapeGeometryDtoCopy, logger);
                Console.WriteLine($"Duration {time}");
                //Assert.IsTrue(time.Max() < 60000);//the default engine timeout
            }
        }
    }
}
