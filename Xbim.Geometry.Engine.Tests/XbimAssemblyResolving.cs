using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace GeometryTests
{
    [TestClass]
    public class XbimAssemblyResolving
    {
        [TestMethod]
        public void Invoking_Geometry_Engine_Should_Work()
        {
            var interopEngine = new Xbim.Geometry.Engine.Interop.XbimGeometryEngine();

            Assert.IsNotNull(interopEngine);

        }
    }
}
