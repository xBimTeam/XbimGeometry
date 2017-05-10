using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.ModelGeometry;

namespace GeometryTests
{
    [TestClass]
    public class EnvironmentTests
    {
        [TestMethod]
        public void TestAvailableRedist()
        {
            Assert.IsTrue(XbimEnvironment.RedistInstalled());

            Assert.IsTrue(
                XbimEnvironment.RedistInstalled(true)
                ||
                XbimEnvironment.RedistInstalled(false)
                );
        }
    }
}
