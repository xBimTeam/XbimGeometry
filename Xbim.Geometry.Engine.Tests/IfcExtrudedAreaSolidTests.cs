using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.ProfileResource;

namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]
    public class IfcExtrudedAreaSolidTests
    {
        [TestMethod]
        public void CompositeProfileDefTest()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m =  IfcStore.Open("SolidTestFiles\\IfcCompositeProfileDefTest.ifc"))
                {
                   
                    var eas = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcCompositeProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IIfcCompositeProfileDef, "Incorrect profiledef found");

                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 40, "IfcCompositeProfileDef profile should have six faces");     

                }
            }

        }

        [TestMethod]
        public void RectangleProfileDefTest()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc"))
                {

                    var eas = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcRectangleProfileDef && !(e.SweptArea is IIfcRectangleHollowProfileDef));
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IIfcRectangleProfileDef, "Incorrect profiledef found");

                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 6, "Rectangular profiles should have six faces");

                    ////now make the extrusion invalid
                    //eas.Depth = 0;
                    //solid = XbimGeometryCreator.CreateSolid(eas);
                    //Assert.IsTrue(eventTrace.Events.Count == 1); //we should have an event here
                    //Assert.IsTrue(Math.Abs(solid.Volume) < m.ModelFactors.Precision);
                    //Assert.IsTrue(solid.BoundingBox.IsEmpty);
                    //Assert.IsFalse(solid.IsValid);

                }
            }
        }

        [TestMethod]
        public void BIM_Logo_LetterB_Test()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\BIM Logo-LetterB.ifc"))
                {                   
                   var eas = m.Instances[88] as IIfcCsgSolid;
                    Assert.IsTrue(eas != null, "No CSG Solid found");

                    var solid = xbimGeometryCreator.Create(eas);
                    Assert.IsTrue(solid is IXbimSolid);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                  //  IfcCsgTests.GeneralTest(solid);
                   // Assert.IsTrue(solid.Faces.Count() == 15, "Letter B should have 15 faces");
                   // var xbimTessellator = new XbimTessellator(m,XbimGeometryType.PolyhedronBinary);
                   // Assert.IsTrue(xbimTessellator.CanMesh(solid));//if we can mesh the shape directly just do it
                   // var shapeGeom = xbimTessellator.Mesh(solid);
                    var shapeGeom = xbimGeometryCreator.CreateShapeGeometry((IXbimSolid)solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance, m.ModelFactors.DeflectionAngle, XbimGeometryType.PolyhedronBinary);

                }
            }
        }
        
       
        [TestMethod]
        public void TestDerivedProfileDefWithTShapedParent()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc"))
                {                  
                    var eas = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcDerivedProfileDef && ((IIfcDerivedProfileDef)e.SweptArea).ParentProfile is IIfcTShapeProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IIfcDerivedProfileDef, "Incorrect profiledef found");
                    Assert.IsTrue(((IIfcDerivedProfileDef)eas.SweptArea).ParentProfile is IIfcTShapeProfileDef, "Incorrect parent profiledef found");

                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 10, "T Shaped profiles should have 10 faces");
                }
            }
        }

        [TestMethod]
        public void ExtrudedCircularProfilesTest()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\15 - Swept pipes.ifc"))
                {
                    
                    foreach (
                        var cp in
                            m.Instances.OfType<IfcExtrudedAreaSolid>().Where(e => e.SweptArea is IfcCircleProfileDef))
                    {
                        Assert.IsTrue(cp != null, "No Extruded Solid found");
                        Assert.IsTrue(cp.SweptArea is IfcCircleProfileDef, "Incorrect profiledef found");

                        var solid = xbimGeometryCreator.CreateSolid(cp);
                        Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                        IfcCsgTests.GeneralTest(solid);

                    }

                    //Assert.IsTrue(solid.Faces.Count() == 10, "T Shaped profiles should have 10 faces");
                }
            }
        }


        [TestMethod]
        public void TestDerivedProfileDefWithLShapedParent()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc"))
                {
                    
                    var eas = m.Instances.OfType<Xbim.Ifc2x3.GeometricModelResource.IfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcDerivedProfileDef && ((IIfcDerivedProfileDef)e.SweptArea).ParentProfile is IIfcLShapeProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IIfcDerivedProfileDef, "Incorrect profiledef found");
                    Assert.IsTrue(((IIfcDerivedProfileDef)eas.SweptArea).ParentProfile is IIfcLShapeProfileDef, "Incorrect parent profiledef found");

                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 8, "L Shaped profiles should have 8 faces");
                }
            }
        }

        [TestMethod]
        public void TestIShapeProfileDef()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc"))
                {

                    var eas = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcIShapeProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IIfcIShapeProfileDef, "Incorrect profiledef found");


                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 14, "I Shaped profiles should have 14 faces");
                }
            }
        }

        [TestMethod]
        public void RectangleHollowProfileDefTest()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc"))
                {
                    var eas = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcRectangleHollowProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IIfcRectangleHollowProfileDef, "Incorrect profiledef found");


                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 10, "Rectangle Hollow profiles should have 10 faces");

                }
            }
        }

        [TestMethod]
        public void TestCircleProfileDef()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\3- IfcExtrudedAreaSolid-IfcCircularProfileDef.ifc"))
                {
                   
                    var eas = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcCircleProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IIfcCircleProfileDef, "Incorrect profiledef found");


                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 3, "Circular  profiles should have 3 faces");
                }
            }
        }
        [TestMethod]
        public void IfcExtrudedArea_With_IfcArbritraryClosedProfileDef_And_IfcCompositeCurve()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\2- IfcExtrudedAreaSolid-IfcArbitraryClosedProfileDef-IfcCompositeCurve.ifc"))
                {

                    var eas = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault();
                    Assert.IsTrue(eas != null);
                    Assert.IsTrue(eas.SweptArea is IIfcArbitraryClosedProfileDef, "Incorrect profile definition");
                    Assert.IsTrue(((IIfcArbitraryClosedProfileDef)eas.SweptArea).OuterCurve is IIfcCompositeCurve, "Incorrect SweptArea type");

                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warnings or errors raised in geometry conversion"); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);

                }
            }
        }

        [TestMethod]
        public void IfcExtrudedArea_With_IfcArbritraryClosedProfileDef_And_IfcPolyline()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc"))
                {
                    var eas = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcArbitraryClosedProfileDef && ((IIfcArbitraryClosedProfileDef)e.SweptArea).OuterCurve is IIfcPolyline);
                    Assert.IsTrue(eas != null);
                    Assert.IsTrue(eas.SweptArea is IIfcArbitraryClosedProfileDef, "Incorrect profile definition");
                    Assert.IsTrue(((IIfcArbitraryClosedProfileDef)eas.SweptArea).OuterCurve is IIfcPolyline, "Incorrect SweptArea outer curve type");

                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warnings or errors raised in geometry conversion"); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    var plineCount = ((IIfcPolyline)((IIfcArbitraryClosedProfileDef)eas.SweptArea).OuterCurve).Points.Count() - 1;
                    Assert.IsTrue(solid.Faces.Count() == plineCount + 2, "IfcPolyline  profiles should have (number of polyline egdes + 2) faces");
                }
            }
        }

        [TestMethod]
        public void IfcExtrudedArea_With_IfcArbritraryProfileDefWithVoids()
        {
            var xbimGeometryCreator = new XbimGeometryEngine();
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = IfcStore.Open("SolidTestFiles\\18 - IfcArbritaryClosedProfileDefWithVoids.ifc"))
                {
                    var eas = m.Instances.OfType<IIfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IIfcArbitraryProfileDefWithVoids);
                    Assert.IsTrue(eas != null);
                    Assert.IsTrue(eas.SweptArea is IIfcArbitraryProfileDefWithVoids, "Incorrect profile definition");

                    var solid = xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warnings or errors raised in geometry conversion"); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);

                    //Assert.IsTrue(solid.Faces.Count() == 6, "IfcPolyline  profiles should have (number of polyline egdes + 2) faces");
                }
            }
        }

    }
}
