﻿using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Logging;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc2x3.Extensions;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.PresentationAppearanceResource;
using Xbim.Ifc2x3.PresentationDefinitionResource;
using Xbim.Ifc2x3.PresentationResource;
using Xbim.Ifc2x3.ProductExtension;
using Xbim.IO;
using Xbim.ModelGeometry.Scene;
using Xbim.XbimExtensions.SelectTypes;
using XbimGeometry.Interfaces;

namespace GeometryTests
{
    /// <summary>
    /// Excutes tests on every geometry item found in every file in the specified directory, passes if no files are found
    /// </summary>
    /// 
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"SolidTestFiles\", "SolidTestFiles")]
    [TestClass]
    public class WholeModelTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();

        [TestMethod]
        public void WriteDpoWOutputFiles()
        {

            const string ifcFileFullName = @"SolidTestFiles\BIM Logo-LetterM.ifc";
            var fileName = Path.GetFileName(ifcFileFullName);
            var fileNameWithoutExtension = Path.GetFileNameWithoutExtension(fileName);
            var workingDir = Path.Combine(Directory.GetCurrentDirectory(), "SolidTestFiles\\");
            var coreFileName = Path.Combine(workingDir, fileNameWithoutExtension);
            var wexBimFileName = Path.ChangeExtension(coreFileName, "wexbim");
            var xbimFile = Path.ChangeExtension(coreFileName, "xbim");
            try
            {

                using (var wexBimFile = new FileStream(wexBimFileName, FileMode.Create))
                {
                    using (var binaryWriter = new BinaryWriter(wexBimFile))
                    {

                        using (var model = new XbimModel())
                        {
                            try
                            {
                                model.CreateFrom(ifcFileFullName, xbimFile, null, true);
                                var geomContext = new Xbim3DModelContext(model);
                                geomContext.CreateContext(XbimGeometryType.PolyhedronBinary);
                                geomContext.Write(binaryWriter);
                            }
                            finally
                            {
                                model.Close();
                                binaryWriter.Flush();
                                wexBimFile.Close();
                            }
                           
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Assert.Fail("Failed to process " + ifcFileFullName + " - " + e.Message);
            }
        }
    



        [TestMethod]
        public void TestAllModels()
        {
            const string modelDirectory = ""; // enter your model directoy here "@"D:\Users\steve\xBIM\Test Models\Use Cases\Live";#
            if (string.IsNullOrWhiteSpace(modelDirectory))
            {
                Trace.WriteLine("TestAllModels tests skipped. Enter a directory where the test models can be found");
                return;
            }
            var di = new DirectoryInfo(modelDirectory);
            FileInfo[] toProcess = di.GetFiles("*.IFC", SearchOption.TopDirectoryOnly);
            foreach (var file in toProcess)
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom(file.FullName, null, null, true, true);

                    using (var eventTrace = LoggerFactory.CreateEventTrace())
                    {
                       
                        foreach (var rep in m.Instances.OfType<IfcGeometricRepresentationItem>())
                        {
                            if (rep is IfcAxis2Placement ||
                                rep is IfcCartesianPoint ||
                                rep is IfcDirection ||
                                rep is IfcCartesianTransformationOperator ||
                                rep is IfcPlanarExtent ||
                                rep is IfcTextLiteralWithExtent ||
                                rep is IfcFillAreaStyleHatching
                                ) continue;
                            var shape = _xbimGeometryCreator.Create(rep);
                          // var w = new XbimOccWriter();
                            //IXbimSolidSet solids = shape as IXbimSolidSet;

                            //foreach (var solid in solids)
                            //{
                            //     w.Write(solid, "d:\\xbim\\f" + i++);
                            //}

                            if (!shape.IsValid)
                                Assert.IsTrue(shape.IsValid, "Invalid shape found");
                            var solid = shape as IXbimSolid;
                            if (solid != null)
                            {
                                   //  w.Write(solid, "d:\\xbim\\x" + rep.EntityLabel.ToString());

                                IfcCsgTests.GeneralTest((IXbimSolid)shape, true, rep is IfcHalfSpaceSolid, rep.EntityLabel);
                            }
                        }
                        if (eventTrace.Events.Count > 0)
                        {
                            //var assertNow = false;
                            Trace.WriteLine("Model: " + file.Name);
                            foreach (var err in eventTrace.Events)
                            {
                                Trace.WriteLine(err.Message);
                                //if (err.EventLevel == EventLevel.ERROR)
                                //    assertNow = true;

                            }

                            // Assert.IsTrue(assertNow == false, "Error events were raised");
                        }

                    }
                }

            }
        }

        /// <summary>
        /// Note this test does not deal with mapped items
        /// </summary>
        [TestMethod]
        public void TestCuttingOpenings()
        {
            //   var w = new XbimOccWriter();
            const string modelDirectory = "";//define where your files to test are @"D:\Users\steve\xBIM\Test Models\Use Cases\Live";
            if (string.IsNullOrWhiteSpace(modelDirectory))
            {
                Trace.WriteLine("TestCuttingOpenings tests skipped. Enter a directory where the test models can be found");
                return;
            }
            var di = new DirectoryInfo(modelDirectory);
            FileInfo[] toProcess = di.GetFiles("*.IFC", SearchOption.TopDirectoryOnly);
            foreach (var file in toProcess)
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom(file.FullName, null, null, true, true);

                    using (var eventTrace = LoggerFactory.CreateEventTrace())
                    {
                        var theElements = _xbimGeometryCreator.CreateSolidSet();
                        var openings = m.Instances.OfType<IfcRelVoidsElement>()
                            .Where(
                                r =>
                                    r.RelatingBuildingElement.Representation != null &&
                                    r.RelatedOpeningElement.Representation != null)
                            .Select(
                                f =>
                                    new
                                    {
                                        element = f.RelatingBuildingElement,
                                        feature = f.RelatedOpeningElement
                                    });

                        var toCut = openings.GroupBy(x => x.element, y => y.feature).ToList();

                        Parallel.ForEach(toCut, new ParallelOptions(), rel =>
                    //         foreach (var rel in toCut)
                        {

                            var elem = rel.Key;
                            var elemTransform = elem.Transform();
                            var elemSolids = _xbimGeometryCreator.CreateSolidSet();
                            foreach (var rep in elem.Representation.Representations)
                            {
                                if (rep.ContextOfItems.ContextType != null &&
                                    "Model" == rep.ContextOfItems.ContextType.Value.ToString() && (rep.RepresentationIdentifier.HasValue && rep.RepresentationIdentifier=="Body"))
                                {
                                    foreach (var item in rep.Items.OfType<IfcGeometricRepresentationItem>())
                                    {
                                        
                                        IXbimGeometryObject shape = _xbimGeometryCreator.Create(item);
                                        if (!shape.IsValid)
                                            Assert.IsTrue(shape.IsValid, "Invalid shape found in #" + item.EntityLabel);
                                        var solid = shape as IXbimSolid;
                                        if (solid != null)
                                        {
                                            IfcCsgTests.GeneralTest(solid, true, item is IfcHalfSpaceSolid,
                                                rep.EntityLabel);

                                            solid = (IXbimSolid)solid.Transform(elemTransform);
                                            elemSolids.Add(solid);
                                        }
                                        else if (shape is IXbimSolidSet)
                                        {
                                            foreach (var subSolid in shape as IXbimSolidSet)
                                            {
                                                IfcCsgTests.GeneralTest(subSolid, true, item is IfcHalfSpaceSolid,
                                                    rep.EntityLabel);
                                                elemSolids.Add(subSolid.Transform(elemTransform));
                                            }
                                        }
                                        else if (shape is IXbimGeometryObjectSet)
                                        {
                                            foreach (var subShape in shape as IXbimGeometryObjectSet)
                                            {
                                                if (subShape is IXbimSolid)
                                                    IfcCsgTests.GeneralTest((IXbimSolid) subShape, true,
                                                        item is IfcHalfSpaceSolid,
                                                        rep.EntityLabel);
                                                elemSolids.Add(subShape.Transform(elemTransform));
                                            }
                                        }
                                        else
                                        {

                                            Trace.WriteLine("No Solid found #" + item.EntityLabel + " " +
                                                            shape.GetType().Name);
                                        }
                                    }
                                }
                            }

                            var openingSolids = _xbimGeometryCreator.CreateSolidSet();
                            foreach (var opening in rel)
                            {
                                var openingTransform = opening.Transform();
                                foreach (var openingrep in opening.Representation.Representations)
                                {
                                    if (openingrep.ContextOfItems.ContextType != null &&
                                        "Model" == openingrep.ContextOfItems.ContextType.Value.ToString() && (openingrep.RepresentationIdentifier.HasValue && openingrep.RepresentationIdentifier == "Body"))
                                    {
                                        foreach (
                                            var openingitem in openingrep.Items.OfType<IfcGeometricRepresentationItem>()
                                            )
                                        {
                                            IXbimGeometryObject openingshape = _xbimGeometryCreator.Create(openingitem);

                                            if (!openingshape.IsValid)
                                            {
                                                Trace.WriteLine("Invalid shape found in #" + openingitem.EntityLabel);
                                                continue;
                                            }
                                            var openingsolid = openingshape as IXbimSolid;
                                            if (openingsolid != null)
                                            {
                                                IfcCsgTests.GeneralTest((IXbimSolid) openingshape, true,
                                                    openingitem is IfcHalfSpaceSolid,
                                                    openingrep.EntityLabel);
                                                openingsolid = (IXbimSolid)openingsolid.Transform(openingTransform);
                                                openingSolids.Add(openingsolid);
                                            }
                                            else if (openingshape is IXbimSolidSet)
                                            {
                                                foreach (var subOpeningSolid in openingshape as IXbimSolidSet)
                                                {
                                                    IfcCsgTests.GeneralTest(subOpeningSolid, true,
                                                        openingitem is IfcHalfSpaceSolid,
                                                        openingrep.EntityLabel);
                                                    openingSolids.Add(subOpeningSolid.Transform(openingTransform));
                                                }
                                            }
                                            else if (openingshape is IXbimGeometryObjectSet)
                                            {
                                                foreach (var subShape in openingshape as IXbimGeometryObjectSet)
                                                {
                                                    if (subShape is IXbimSolid)
                                                        IfcCsgTests.GeneralTest((IXbimSolid) subShape, true,
                                                            openingitem is IfcHalfSpaceSolid,
                                                            openingrep.EntityLabel);
                                                    openingSolids.Add(subShape.Transform(openingTransform));
                                                }
                                            }
                                            else
                                            {
                                                Trace.WriteLine("No Solid found #" + openingitem.EntityLabel + " " +
                                                                openingshape.GetType().Name);
                                            }
                                        }
                                    }
                                }
                            }

                            // ReSharper disable once AccessToDisposedClosure

                            var solidResult = elemSolids.Cut(openingSolids, m.ModelFactors.PrecisionBoolean);
                            foreach (var result in solidResult)
                            {
                                IfcCsgTests.GeneralTest(result, true, false,
                                    elem.EntityLabel);
                                theElements.Add(result);
                            }

                        }
                        );
                       // w.Write(theElements, "d:\\xbim\\r");
                        if (eventTrace.Events.Count > 0)
                        {
                            //var assertNow = false;
                            Trace.WriteLine("Model: " + file.Name);
                            foreach (var err in eventTrace.Events)
                            {
                                Trace.WriteLine(err.Message);
                                //if (err.EventLevel == EventLevel.ERROR)
                                //    assertNow = true;

                            }

                            // Assert.IsTrue(assertNow == false, "Error events were raised");
                        }
                    }
                }
            }

        }
    }
}


