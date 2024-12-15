using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Drawing;
using System.Linq;
using Xbim.Common.Configuration;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class GithubIssuesTests
    {
        [TestMethod]
        public void Github_Issue_281()
        {
            // this file resulted in a stack-overflow exception due to precision issues in the data.
            // We have added better exception management so that the stack-overflow is not thrown any more, 
            // however the voids in the wall are still not computed correctly.
            //
            using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
            {
                m.LoadStep21("TestFiles\\Github\\Github_issue_281_minimal.ifc");
                var c = new Xbim3DModelContext(m);
                c.CreateContext(null, false);

                // todo: 2021: add checks so that the expected openings are correctly computed.
            }
        }

        [TestMethod]
        public void Github_Issue_447()
        {
            // This file contains a trimmed curve based on ellipse which has semiaxis1 < semiaxis2
            // and trimmed curve is parameterized with cartesian points.
            // This test checks for a bug in XBimCurve geometry creation procedure when incorrect parameter values
            // are calculated for these specific conditions described above.
            using (var model = MemoryModel.OpenRead(@"TestFiles\Github\Github_issue_447.ifc"))
            {
                var shape = model.Instances.OfType<IIfcTrimmedCurve>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var trimPoint1 = shape.Trim1.OfType<IIfcCartesianPoint>().FirstOrDefault();
                Assert.IsNotNull(trimPoint1);
                var trimPoint2 = shape.Trim2.OfType<IIfcCartesianPoint>().FirstOrDefault();
                Assert.IsNotNull(trimPoint2);

                var trimStart = new XbimPoint3D(trimPoint1.X, trimPoint1.Y, trimPoint1.Z);
                var trimEnd = new XbimPoint3D(trimPoint2.X, trimPoint2.Y, trimPoint2.Z);

                IXbimGeometryEngine geomEngine = new XbimGeometryEngine();
                var geom = geomEngine.CreateCurve(shape);
                Assert.IsNotNull(geom);

                Assert.AreEqual(trimStart, geom.Start);
                Assert.AreEqual(trimEnd, geom.End);
            }
        }

        [TestMethod]
        public void Github_Issue_512()
        {
            //var loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            //XbimServices.Current.ConfigureServices(s => s.AddXbimToolkit(b => b.AddLoggerFactory(loggerFactory)));
            var ifcFile = @"TestFiles\Github\Github_issue_512.ifc";
            // Triggers OCC Memory violation
            using (var m = MemoryModel.OpenRead(ifcFile))
            {
                var c = new Xbim3DModelContext(m);
                var result = c.CreateContext(null, true);

                Assert.IsTrue(result, "Expect success");

                Assert.IsFalse(m.GeometryStore.IsEmpty, "Store expected to be full");
            }
        }

        [TestMethod]
        public void Github_Issue_512b()
        {
            //var loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            //Common.Configuration.XbimServices.Current.ConfigureServices(s => s.AddXbimToolkit(b => b.AddLoggerFactory(loggerFactory)));
            var ifcFile = @"TestFiles\Github\Github_issue_512b.ifc";
            // Triggers OCC Memory violation
            using (var m = MemoryModel.OpenRead(ifcFile))
            {
                var c = new Xbim3DModelContext(m);
                var result = c.CreateContext(null, true);

                Assert.IsTrue(result, "Expect success");

                Assert.IsFalse(m.GeometryStore.IsEmpty, "Store expected to be full");

                using (var reader = m.GeometryStore.BeginRead())
                {
                    var regions = reader.ContextRegions.Where(cr => cr.MostPopulated() != null).Select(cr => cr.MostPopulated());

                    var region = regions.FirstOrDefault();

                    region.Size.Length.Should().BeApproximately(1.747, 0.001);
                }
            }
        }

        //[TestMethod]

        //public void Github_Issue_512d()
        //{

        //    var ifcFile = @"C:\Users\AndyWard\XBIM\Models - Documents\0400 Under NDA\TraceSoftware\CDO_EXE_240_ML04_S_1_01.extracted.ifc";
        //    // Triggers OCC Memory violation
        //    using (var m = MemoryModel.OpenRead(ifcFile))
        //    {
        //        var loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
        //        var geomEngine = new XbimGeometryEngine();
        //        var logger = loggerFactory.CreateLogger<GithubIssuesTests>();

        //        IIfcAdvancedBrep brep = m.Instances[9020] as IIfcAdvancedBrep;

        //        var geom = geomEngine.CreateSolidSet(brep, logger);

        //        geom.IsValid.Should().BeTrue();

        //        foreach(var shape in geom)
        //        {
        //            var bb = shape.BoundingBox;
        //            Console.WriteLine($"{shape.IsValid} {shape.IsPolyhedron} {shape.BoundingBox.Length()} {shape.BoundingBox} ");
        //        }
        //       // geom.SaveAsBrep("Foo.brep");

        //        geom.BoundingBox.Length().Should().BeLessOrEqualTo(1e10);
        //    }

        //}
    }
}
