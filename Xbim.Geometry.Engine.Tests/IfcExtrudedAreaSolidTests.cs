using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Geometry.Engine.Interop;
using Xbim.IO;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.ProfileResource;
using Xbim.Common.Logging;
using Xbim.Ifc2x3.GeometryResource;
using XbimGeometry.Interfaces;


namespace GeometryTests
{
    
    [TestClass]
    [DeploymentItem(@"SolidTestFiles\")]
   
    public class IfcExtrudedAreaSolidTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void RectangleProfileDefTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea.GetType()== typeof(IfcRectangleProfileDef));
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IfcRectangleProfileDef, "Incorrect profiledef found");
                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
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
        public void TestDerivedProfileDefWithTShapedParent()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IfcDerivedProfileDef && ((IfcDerivedProfileDef)e.SweptArea).ParentProfile is IfcTShapeProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IfcDerivedProfileDef, "Incorrect profiledef found");
                    Assert.IsTrue(((IfcDerivedProfileDef)eas.SweptArea).ParentProfile is IfcTShapeProfileDef, "Incorrect parent profiledef found");
                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 10, "T Shaped profiles should have 10 faces");
                }
            }
        }

        [TestMethod]
        public void ExtrudedCircularProfilesTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("15 - Swept pipes.ifc", null, null, true, true);
                    foreach (
                        var cp in
                            m.Instances.OfType<IfcExtrudedAreaSolid>().Where(e => e.SweptArea is IfcCircleProfileDef))
                    {
                        Assert.IsTrue(cp != null, "No Extruded Solid found");
                        Assert.IsTrue(cp.SweptArea is IfcCircleProfileDef, "Incorrect profiledef found");
                        
                        var solid = _xbimGeometryCreator.CreateSolid(cp);
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
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IfcDerivedProfileDef && ((IfcDerivedProfileDef)e.SweptArea).ParentProfile is IfcLShapeProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IfcDerivedProfileDef, "Incorrect profiledef found");
                    Assert.IsTrue(((IfcDerivedProfileDef)eas.SweptArea).ParentProfile is IfcLShapeProfileDef, "Incorrect parent profiledef found");
                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                     IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 8, "L Shaped profiles should have 8 faces");
                }
            }
        }

        [TestMethod]
        public void TestIShapeProfileDef()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IfcIShapeProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IfcIShapeProfileDef, "Incorrect profiledef found");
                    
                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 14, "I Shaped profiles should have 14 faces");
                }
            }
        }

        [TestMethod]
        public void RectangleHollowProfileDefTest()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IfcRectangleHollowProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IfcRectangleHollowProfileDef, "Incorrect profiledef found");

                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 10, "Rectangle Hollow profiles should have 10 faces");
                   
                }
            }
        }

 [TestMethod]
        public void TestCircleProfileDef()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("3- IfcExtrudedAreaSolid-IfcCircularProfileDef.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IfcCircleProfileDef);
                    Assert.IsTrue(eas != null, "No Extruded Solid found");
                    Assert.IsTrue(eas.SweptArea is IfcCircleProfileDef, "Incorrect profiledef found");

                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 3, "Circular  profiles should have 3 faces");
                }
            }
        }
        [TestMethod]
        public void IfcExtrudedArea_With_IfcArbritraryClosedProfileDef_And_IfcCompositeCurve()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("2- IfcExtrudedAreaSolid-IfcArbitraryClosedProfileDef-IfcCompositeCurve.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault();
                    Assert.IsTrue(eas != null);
                    Assert.IsTrue(eas.SweptArea is IfcArbitraryClosedProfileDef,"Incorrect profile definition");
                    Assert.IsTrue(((IfcArbitraryClosedProfileDef)eas.SweptArea).OuterCurve is IfcCompositeCurve, "Incorrect SweptArea type");
                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0,"Warnings or errors raised in geometry conversion"); //no events should have been raised from this call

                     IfcCsgTests.GeneralTest(solid);

                    //now make the extrusion invalid
                    eas.Depth = 0;
                    solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 1,"An expected error was not raised raised in geometry conversion"); //we should have an event here
                    Assert.IsTrue(Math.Abs(solid.Volume) < m.ModelFactors.Precision);
                    Assert.IsTrue(solid.BoundingBox.IsEmpty);
                    Assert.IsFalse(solid.IsValid);

                }
            }
        }

        [TestMethod]
        public void IfcExtrudedArea_With_IfcArbritraryClosedProfileDef_And_IfcPolyline()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("1- IfcExtrudedAreaSolid-IfcProfileDef-Parameterised.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IfcArbitraryClosedProfileDef && ((IfcArbitraryClosedProfileDef)e.SweptArea).OuterCurve is IfcPolyline);
                    Assert.IsTrue(eas != null);
                    Assert.IsTrue(eas.SweptArea is IfcArbitraryClosedProfileDef, "Incorrect profile definition");
                    Assert.IsTrue(((IfcArbitraryClosedProfileDef)eas.SweptArea).OuterCurve is IfcPolyline, "Incorrect SweptArea outer curve type");
                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warnings or errors raised in geometry conversion"); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    var plineCount = ((IfcPolyline)((IfcArbitraryClosedProfileDef)eas.SweptArea).OuterCurve).Points.Count-1;
                    Assert.IsTrue(solid.Faces.Count() == plineCount + 2, "IfcPolyline  profiles should have (number of polyline egdes + 2) faces");
                }
            }
        }

        [TestMethod]
        public void IfcExtrudedArea_With_IfcArbritraryProfileDefWithVoids()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("18 - IfcArbritaryClosedProfileDefWithVoids.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault(e => e.SweptArea is IfcArbitraryProfileDefWithVoids);
                    Assert.IsTrue(eas != null);
                    Assert.IsTrue(eas.SweptArea is IfcArbitraryProfileDefWithVoids, "Incorrect profile definition");
                  
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0, "Warnings or errors raised in geometry conversion"); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                   
                    //Assert.IsTrue(solid.Faces.Count() == 6, "IfcPolyline  profiles should have (number of polyline egdes + 2) faces");
                }
            }
        }
       
    }
}
