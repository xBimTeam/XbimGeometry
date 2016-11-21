using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.ProfileResource;
using Xbim.IO;

namespace GeometryTests
{
    [TestClass]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"MaintenanceTestFiles\", "MaintenanceTestFiles")]
    public class MaintenanceTests
    {

        [TestMethod]
        public void IfcPolygonalBoundedHalfSpace_Open()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom(@"MaintenanceTestFiles\Recoverable.IFCPOLYGONALBOUNDEDHALFSPACE.ifc", null, null, true, true);
                    var pbhss = m.Instances[43276] as IfcPolygonalBoundedHalfSpace;

                    xbimGeometryCreator.CreateSolid(pbhss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                }
            }
        }

        [TestMethod]
        public void IfcPolygonalBoundedHalfSpace_Closed()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom(@"MaintenanceTestFiles\ok.IFCPOLYGONALBOUNDEDHALFSPACE.ifc", null, null, true, true);
                    var pbhss = m.Instances[43290] as IfcPolygonalBoundedHalfSpace;

                    xbimGeometryCreator.CreateSolid(pbhss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                }
            }
        }
    }
}
