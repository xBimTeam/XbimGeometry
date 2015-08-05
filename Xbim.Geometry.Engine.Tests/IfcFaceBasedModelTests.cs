using System.IO;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc2x3.TopologyResource;
using Xbim.IO;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Common.Logging;
using Xbim.Ifc2x3.GeometricConstraintResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.ProductExtension;
using XbimGeometry.Interfaces;

namespace GeometryTests
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
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\Complex_BRep_Boolean.ifc", null, null, true, true);
                    var fbr = m.Instances[35] as IfcFacetedBrep;
                    Assert.IsTrue(fbr != null, "No IfcFacetedBRep found");
                    var bodyShape = (IXbimShell)((IXbimGeometryObjectSet)_xbimGeometryCreator.Create(fbr)).First();
                    Assert.IsTrue(bodyShape.IsValid, "Invalid IfcFacetedBRep");
                    var opening = m.Instances[133218] as IfcExtrudedAreaSolid;
                    Assert.IsTrue(opening != null, "No IfcExtrudedAreaSolid found");
                    var window = m.Instances[133212] as IfcOpeningElement;
                    var cutShape = (IXbimSolid)_xbimGeometryCreator.Create(opening, (IfcAxis2Placement3D)((IfcLocalPlacement)window.ObjectPlacement).RelativePlacement);
                    var result =  bodyShape.Cut(cutShape, m.ModelFactors.OneMilliMetre);
                }
            }
        }



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
        public void IfcFacetedBRepTubeModelTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\IfcFacetedBRepWithIncorrectlyOrientedFaces.ifc", null, null, true, true);
                    var fbr = m.Instances[98711] as IfcFacetedBrep;
                    Assert.IsTrue(fbr != null, "No IfcFacetedBRep found");
                    int faceCount = fbr.Outer.CfsFaces.Count;

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
