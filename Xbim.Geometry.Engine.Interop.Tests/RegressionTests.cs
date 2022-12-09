using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using System.Diagnostics;
using System.Linq;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xunit;
namespace Xbim.Geometry.Engine.Interop.Tests
{
    
    public class RegressionTests
    {
        
        
 
        static private ILogger logger = NullLogger< RegressionTests >.Instance;
        static private ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        

        // todo: 2021: @SRL this test used to be ignored, but the reason is not clear
        [Fact]
        public void IfcHalfspace_FailingGeom()
        {
            using (var m = IfcStore.Open("TestFiles\\Regression\\FailingGeom.ifc"))
            {
                var geomEngine = new XbimGeometryEngine(m, loggerFactory);
                var extSolid = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(hs => hs.EntityLabel == 185025);
                var solid = geomEngine.CreateSolid(extSolid, logger);
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
                    var ret = geomEngine.CreateSolidSet(eas, logger);

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
