using System;
using System.Linq;
using Ifc4GeometryTests;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.ModelGeometry.Scene;

namespace GeometryTests
{

    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]
    public class CatiaIfcOpenShellTest
    {

        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();

        /// <summary>
        /// the following test showed an error in the ifc file produced by Catia, they incorrectly pointed a product shape at an IfcOpenShell, rather than a IFCSHELLBASEDSURFACEMODEL
        /// </summary>
        [TestMethod]
        [DeploymentItem(@"SolidTestFiles\CatiaIfcOpenShellTest.ifc")]
        public void CanMeshIfcOpenShell()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("CatiaIfcOpenShellTest.ifc"))
                {
                    var context = new Xbim3DModelContext(m);
                    context.CreateContext();
                    var c = context.ShapeInstances().Count();
                    Assert.AreEqual(1, c, "Count is not one.");
                }
            }
        }
    }
}
