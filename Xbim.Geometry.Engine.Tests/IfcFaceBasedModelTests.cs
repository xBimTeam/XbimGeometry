using System.IO;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]
    public class IfcFaceBasedModelTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();

        /// <summary>
        /// Cut a set of brep faces without forming a solid
        /// </summary>
        [TestMethod]
        public void CutNonSolidBrep()
        {

            using (var m = IfcStore.Open("SolidTestFiles\\Complex_BRep_Boolean.ifc"))
            {
                var fbr = m.Instances[35] as IIfcFacetedBrep;
                Assert.IsTrue(fbr != null, "No IfcFacetedBRep found");
                var bodyShape = (IXbimGeometryObjectSet)_xbimGeometryCreator.Create(fbr);
                Assert.IsTrue(bodyShape.IsValid, "Invalid IfcFacetedBRep");
                var shell = bodyShape.Shells.First;
                var opening = m.Instances[133218] as IIfcExtrudedAreaSolid;
                Assert.IsTrue(opening != null, "No IfcExtrudedAreaSolid found");
                var window = m.Instances[133212] as IIfcOpeningElement;
                if (window != null)
                {
                    var cutShape = (IXbimSolid)_xbimGeometryCreator.Create(opening, (IIfcAxis2Placement3D)((IIfcLocalPlacement)window.ObjectPlacement).RelativePlacement);
                    shell.Cut(cutShape, m.ModelFactors.OneMilliMetre);
                }
            }

        }

        

        /// <summary>
        /// This is a bad Brep that does not correctly define the faceset
        /// </summary>
        [TestMethod]
        public void IfcFacetedBRepWithMultipleSolids()
        {

            using (var m = IfcStore.Open("SolidTestFiles\\12 - Multiple FacetedBrep.ifc"))
            {
                var fbr = m.Instances[25] as IIfcFacetedBrep;
                Assert.IsTrue(fbr != null, "No IfcFacetedBRep found");

                var solids = _xbimGeometryCreator.CreateSolidSet(fbr);

                Assert.IsTrue(solids.Count == 11, "Expected 11 solids");
                foreach (var solid in solids)
                {
                    IfcCsgTests.GeneralTest(solid, true);
                }
            }

        }

        [TestMethod]
        public void IIfcFaceBasedSurfaceModelAsSolid()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\FacebasedModelWithMissingFace.ifc"))
                {
                    var fbsm = m.Instances[154529] as IIfcFaceBasedSurfaceModel;
                    Assert.IsTrue(fbsm != null, "No IIfcFaceBasedSurfaceModel found");
                    int faceCount = fbsm.FbsmFaces.Count();
                    
                    var surface = _xbimGeometryCreator.CreateSurfaceModel(fbsm);
                    var solid = _xbimGeometryCreator.CreateSolidSet();
                    solid.Add(surface);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warning or Error events were raised"); //we should have no warnings
                    Assert.IsTrue(solid.FirstOrDefault()!=null);                   
                    IfcCsgTests.GeneralTest(solid.First);                
                }
            }
        }
        [TestMethod]
        public void IfcFacetedBRepTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\12 - Multiple FacetedBrep.ifc"))
                {
                    var fbr = m.Instances[780] as IIfcFacetedBrep;
                    Assert.IsTrue(fbr != null, "No IfcFacetedBRep found");
                    int faceCount = fbr.Outer.CfsFaces.Count();
                    
                    var solids = _xbimGeometryCreator.CreateSolidSet(fbr);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warning or Error events were raised"); //we should have no warnings
                    
                    Assert.IsTrue(solids.Count == 1, "Expected 11 solids");
                    
                    IfcCsgTests.GeneralTest(solids.First);
                    
                    Assert.IsTrue(solids.First.Faces.Count() == faceCount, "Failed to convert all faces");
                }
            }
        }

        [TestMethod]
        public void IfcFacetedBRepTubeModelTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\IfcFacetedBRepWithIncorrectlyOrientedFaces.ifc"))
                {
                    var fbr = m.Instances[98711] as IIfcFacetedBrep;
                    Assert.IsTrue(fbr != null, "No IfcFacetedBRep found");

                    var compound = _xbimGeometryCreator.Create(fbr);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warning or Error events were raised"); //we should have no warnings
                    var bw = new BinaryWriter(new MemoryStream());
                    _xbimGeometryCreator.WriteTriangulation(bw,compound, m.ModelFactors.Precision,m.ModelFactors.DeflectionTolerance);
                    //Assert.IsTrue(solids.Count == 1, "Expected 1 solid");

                    //IfcCsgTests.GeneralTest(solids.First);

                    //Assert.IsTrue(solids.First.Faces.Count() == faceCount, "Failed to convert all faces");
                }
            }
        }

        [TestMethod]
        public void IfcShellBasedSurfaceModelTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\16 - IfcShellBasedSurfaceModel.ifc"))
                {
                    var sbsm = m.Instances[38] as IIfcShellBasedSurfaceModel;
                    Assert.IsTrue(sbsm != null, "No IfcShellBasedSurfaceModel found");
                    Assert.IsTrue(sbsm.SbsmBoundary.First() is IIfcOpenShell, "ifc OpenShell not found");
                    
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
                using (var m = IfcStore.Open("SolidTestFiles\\17 - IfcFaceBasedSurfaceModel.ifc"))
                {
                    var fbsm = m.Instances[29] as IIfcFaceBasedSurfaceModel;
                    Assert.IsTrue(fbsm != null, "No IfcFaceBasedSurfaceModel found");

                    
                    _xbimGeometryCreator.CreateSurfaceModel(fbsm);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warning or Error events were raised");
                        //we should have no warnings
                   
                }
            }
        }
    }
}
