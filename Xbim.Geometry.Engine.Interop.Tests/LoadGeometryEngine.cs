using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class LoadGeometryEngine
    {
        [TestMethod]
        public void SimpleLoad()
        {
            var ge = new XbimGeometryEngine();
            Assert.IsNotNull(ge);

        }
        [TestMethod]
        public void TestLogging()
        {
            //Xbim.Common.ApplicationLogging.LoggerFactory.AddConsole(LogLevel.Trace);
            //var ge = new XbimGeometryEngine();
            
            //ge.Logger.LogTrace("Lets see if we trace what logging does");
            //ge.Logger.LogDebug("Trying to debug logging");
            //ge.Logger.LogInformation("I am informing you before I warn you");
            //ge.Logger.LogWarning("I have warned you!");
            //ge.Logger.LogError("Sorry I am in error");
            //ge.Logger.LogCritical("It was a critical mistake");

        }
    }
}
