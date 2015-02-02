using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc2x3.TopologyResource;
using Xbim.IO;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Common.Logging;

namespace GeometryTests
{
    [TestClass]
    public class IfcFaceBasedModelTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        /// <summary>
        /// This is a bad Brep that does not correctly define the faceset
        /// </summary>
        [TestMethod]
        public void IfcFacetedBRepWithMultipleSolids()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\12 - Multiple FacetedBrep.ifc", null, null, true, true);
                    var fbr = m.Instances[25] as IfcFacetedBrep;
                    Assert.IsTrue(fbr != null, "No IfcFacetedBRep found");
                    int faceCount = fbr.Outer.CfsFaces.Count;
                    
                    var solids = _xbimGeometryCreator.CreateSolidSet(fbr);

                    Assert.IsTrue(solids.Count == 11, "Expected 11 solids");
                    foreach (var solid in solids)
                    {
                        IfcCsgTests.GeneralTest(solid, true);
                    }
                }
            }
        }

        [TestMethod]
        public void IfcFacetedBRepTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\12 - Multiple FacetedBrep.ifc", null, null, true, true);
                    var fbr = m.Instances[780] as IfcFacetedBrep;
                    Assert.IsTrue(fbr != null, "No IfcFacetedBRep found");
                    int faceCount = fbr.Outer.CfsFaces.Count;
                    
                    var solids = _xbimGeometryCreator.CreateSolidSet(fbr);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warning or Error events were raised"); //we should have no warnings
                    
                    Assert.IsTrue(solids.Count == 1, "Expected 11 solids");
                    
                    IfcCsgTests.GeneralTest(solids.First);
                    
                    Assert.IsTrue(solids.First.Faces.Count() == faceCount, "Failed to convert all faces");
                }
            }
        }

        [TestMethod]
        public void IfcShellBasedSurfaceModelTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\16 - IfcShellBasedSurfaceModel.ifc", null, null, true, true);
                    var sbsm = m.Instances[38] as IfcShellBasedSurfaceModel;
                    Assert.IsTrue(sbsm != null, "No IfcShellBasedSurfaceModel found");
                    Assert.IsTrue(sbsm.SbsmBoundary.First is IfcOpenShell, "ifc OpenShell not found");
                    
                    _xbimGeometryCreator.CreateSurfaceModel(sbsm);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warning or Error events were raised");
                        //we should have no warnings
                   
                }
            }
        }
        
        [TestMethod]
        public void IfcFaceBasedSurfaceModelTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\17 - IfcFaceBasedSurfaceModel.ifc", null, null, true, true);
                    var fbsm = m.Instances[29] as IfcFaceBasedSurfaceModel;
                    Assert.IsTrue(fbsm != null, "No IfcFaceBasedSurfaceModel found");

                    
                    _xbimGeometryCreator.CreateSurfaceModel(fbsm);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warning or Error events were raised");
                        //we should have no warnings
                   
                }
            }
        }
    }
}
