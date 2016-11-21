using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.ProfileResource;
using Xbim.IO;

namespace GeometryTests
{
    [TestClass]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"SolidTestFiles\ifcpolygonalboundedhalfspace.open.ifc", @"SolidTestFiles")]
    [DeploymentItem(@"SolidTestFiles\ifcpolygonalboundedhalfspace.open.ifc", @"SolidTestFiles")]
    public class IfcPolygonalBoundedHalfSpaceTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void IfcPolygonalBoundedHalfSpace_Open()
        {
            DirectoryInfo d = new DirectoryInfo(".");
            var a = d.FullName;
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open(@"SolidTestFiles\ifcpolygonalboundedhalfspace.open.ifc"))
                {    
                    var pbhss = m.Instances[43276] as IfcPolygonalBoundedHalfSpace;
                    _xbimGeometryCreator.CreateSolid(pbhss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                }
            }
        }

        [TestMethod]
        public void IfcPolygonalBoundedHalfSpace_Closed()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open(@"SolidTestFiles\ifcpolygonalboundedhalfspace.closed.ifc"))
                {
                    var pbhss = m.Instances[43290] as IfcPolygonalBoundedHalfSpace;

                    _xbimGeometryCreator.CreateSolid(pbhss);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                }
            }
        }
    }
}
