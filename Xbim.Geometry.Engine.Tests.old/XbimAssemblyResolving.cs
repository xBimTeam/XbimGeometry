using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Ifc4GeometryTests
{
    [TestClass]
    public class XbimAssemblyResolving
    {
        [TestMethod]
        public void Invoking_Geometry_Engine_Should_Work()
        {
            var interopEngine = new Xbim.Geometry.Engine.Interop.XbimGeometryEngine();
            // We're not expecting an exception
            Assert.IsNotNull(interopEngine);
            
        }
    }
}
