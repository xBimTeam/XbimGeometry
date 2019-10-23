using System;
using System.Linq;
using GeometryTests;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
using Xbim.Ifc;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.Interfaces;


namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [TestClass]
    public class Ifc2x3GeometryTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();

        #region Grid 

        /// <summary>
        /// Test case introduced to address the issue presented at
        /// https://github.com/xBimTeam/XbimWindowsUI/issues/58
        /// the test file has been reduced to fit in the test suite, but the original submission is available online if needed.
        /// </summary>
        [TestMethod]
        [DeploymentItem(@"SolidTestFiles\Ifc2x3grid.ifc")]
        public void ArchicadFailureCaseGridCreation()
        {
            using (var model = IfcStore.Open(@"Ifc2x3grid.ifc"))
            {

                var ifcGrid = model.Instances.OfType<IIfcGrid>().FirstOrDefault();
                Assert.IsNotNull(ifcGrid);
                var geom = _xbimGeometryCreator.CreateGrid(ifcGrid);
                foreach (var solid in geom)
                {
                    Assert.IsTrue(solid.Faces.Count == 6, "Each grid line must have six faces");
                }
            }
        }


        #endregion
        
    }
}
