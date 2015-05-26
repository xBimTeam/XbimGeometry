using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.IO;

namespace GeometryTests
{
    [TestClass]
    [DeploymentItem(@"EsentTestFiles\")]
    public class XbimModelExtensionsTests
    {
        [TestMethod]
        public void CanGetRawGeometry()
        {
            var m = new XbimModel();
            m.Open("Monolith_v20.xBIM");
            Assert.AreEqual(2, m.GeometrySupportLevel, "GeometrySupportLevel for Monolith_v20 should be 2");
            m.Close();
        }
    }
}
