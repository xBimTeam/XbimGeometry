using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting.Logging;
using System;
using System.IO;
using System.Reflection;
using Xbim.Ifc4.GeometryResource;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class LoadGeometryEngine
    {
        [TestMethod]
        public void SimpleLoad()
        {
            var mm = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3());
            var logger = new NullLogger<LoadGeometryEngine>();
            var ge = new XbimGeometryEngine(mm,logger);
            Assert.IsNotNull(ge);

        }

       

        [TestMethod]
        public void TestLogging()
        {


            var logger = new NullLogger<IfcAdvancedBrepTests>();

           
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                var ge = new XbimGeometryEngine(m,logger);
                using (var txn = m.BeginTransaction("new"))
                {
                    var pline = m.Instances.New<IfcPolyline>();
                    ge.CreateCurve(pline, logger);
                }

            }
        }

       
    }
}
