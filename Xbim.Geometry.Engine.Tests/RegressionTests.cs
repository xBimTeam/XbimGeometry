using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.Diagnostics;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;
namespace Xbim.Geometry.Engine.Tests
{

    public class RegressionTests
    {
        private readonly ILoggerFactory _loggerFactory;
        private ILogger _logger;

        public RegressionTests(ILoggerFactory loggerFactory)
        {
            _logger= loggerFactory.CreateLogger<RegressionTests>();
            _loggerFactory = loggerFactory;
        }

        

        // todo: 2021: @SRL this test used to be ignored, but the reason is not clear
        [Fact]
        public void IfcHalfspace_FailingGeom()
        {
            using (var m = MemoryModel.OpenRead("TestFiles\\Regression\\FailingGeom.ifc"))
            {
                var geomEngine = new XbimGeometryEngine(m, _loggerFactory);
                var extSolid = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(hs => hs.EntityLabel == 185025);
                var solid = geomEngine.CreateSolid(extSolid, _logger);
                HelperFunctions.GeneralTest(solid);

                var mlist = m.Instances.OfType<IIfcBooleanClippingResult>();
                foreach (var eas in mlist)
                {
                    Debug.WriteLine("Todo: " + eas.EntityLabel);
                }

                foreach (var eas in mlist)
                {
                    // var eas = m.Instances.OfType<IIfcBooleanClippingResult>().FirstOrDefault(hs => hs.EntityLabel == 185249);
                    eas.SecondOperand.Should().NotBeNull();
                    var ret = geomEngine.CreateSolidSet(eas, _logger);

                    HelperFunctions.GeneralTest(solid);

                    Debug.WriteLine(eas.EntityLabel + " ok");
                    //if (eas.EntityLabel == 185243)
                    //{
                    //    File.WriteAllText(@"C:\Data\_tmp\185243.v5.brep", solid.ToBRep);
                    //}
                }
            }
        }
    }
}
