using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Xbim.Common.Geometry;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.ProfileResource;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
	[TestClass]
	public class ShapeAdjustementTests
	{
        static private IXbimGeometryEngine geomEngine;
        static private ILoggerFactory loggerFactory;
        static private ILogger logger;

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<Ifc4GeometryTests>();
        }

        [ClassCleanup]
        static public void Cleanup()
        {
            loggerFactory = null;
            geomEngine = null;
            logger = null;
        }

        [TestMethod]
        public void DuplicatePointPolygonCleanup()
        {
            List<XbimPoint3D> ps = new List<XbimPoint3D>();
            ps.Add(new XbimPoint3D(0, 0, 0));
            ps.Add(new XbimPoint3D(10, 0, 0));
            ps.Add(new XbimPoint3D(10, 0.000001, 0)); // this point is same as previous
            ps.Add(new XbimPoint3D(10, 10, 0));
            ps.Add(new XbimPoint3D(0, 10, 0));
            Check(ps, 4);

            ps = new List<XbimPoint3D>();
            ps.Add(new XbimPoint3D(0, 0, 0));
            ps.Add(new XbimPoint3D(0, 0.000001, 0)); // we expect this to disappear, not the 0,0,0 because of consistency of the endpoint
            ps.Add(new XbimPoint3D(10, 0, 0));
            ps.Add(new XbimPoint3D(10, 0.000001, 0)); // this point is same as previous, should be removed
            ps.Add(new XbimPoint3D(10, 10, 0));
            ps.Add(new XbimPoint3D(0, 10, 0));
            ps.Add(new XbimPoint3D(0, 0.000001, 0)); // we expect this to disappear, not the 0,0,0 because of consistency of the first point in closed loops
            var returnedbrep = Check(ps, 4);
            Assert.IsFalse(returnedbrep.Contains(".000001"), "because it removed the wrong point");
        }

        [TestMethod]
		public void PolygonAlignedCleanup()
		{
            /*
			 * From PR 301
			 * 
			#172= IFCCARTESIANPOINT((3062.13203435597,-150.));
			#174= IFCCARTESIANPOINT((3362.13203435597,150.));
			#176= IFCCARTESIANPOINT((2937.86796564404,150.));
			#178= IFCCARTESIANPOINT((0.,150.));
			#180= IFCCARTESIANPOINT((0.,-150.));
			#182= IFCPOLYLINE((#172,#174,#176,#178,#180,#172));
			#184= IFCARBITRARYCLOSEDPROFILEDEF(.AREA.,$,#182);
			#187= IFCAXIS2PLACEMENT3D(#6,$,$);
			#188= IFCEXTRUDEDAREASOLID(#184,#187,#20,3000.); 
			 */

            List<XbimPoint3D> ps = new List<XbimPoint3D>();
            ps.Add(new XbimPoint3D(3062.13203435597,-150,0));
            ps.Add(new XbimPoint3D(3362.13203435597,150,0));
            ps.Add(new XbimPoint3D(2937.86796564404,150,0)); // this point is aligned with the surrounding one, could be removed
            ps.Add(new XbimPoint3D(0,150,0));
            ps.Add(new XbimPoint3D(0,-150,0));
            Check(ps, 4);



            ps = new List<XbimPoint3D>();
            ps.Add(new XbimPoint3D(0, 0, 0));
            ps.Add(new XbimPoint3D(10, 0, 0));
            ps.Add(new XbimPoint3D(10, 10, 0)); 
            ps.Add(new XbimPoint3D(5.000001, 10, 0)); // this should be removed because aligned
            ps.Add(new XbimPoint3D(0, 10, 0));
            var returnedbrep = Check(ps, 4);
            Assert.IsFalse(returnedbrep.Contains(".000001"), "because it removed the wrong point"); // should not find the point with .00001 coordinate

            // more complex case with overlapping wires
            ps = new List<XbimPoint3D>();
            ps.Add(new XbimPoint3D(0, 0, 0));
            ps.Add(new XbimPoint3D(10, 0, 0));
            ps.Add(new XbimPoint3D(10, 10, 0));
            ps.Add(new XbimPoint3D(3.000001, 10, 0)); // this should be removed because aligned
            ps.Add(new XbimPoint3D(7.000001, 10, 0)); // this goes in the opposite direction of the previous segment
            ps.Add(new XbimPoint3D(0, 10, 0));
            returnedbrep = Check(ps, 4);
            Assert.IsFalse(returnedbrep.Contains(".000001"), "because it removed the wrong point"); // should not find the point with .00001 coordinate

        }

        private string Check(List<XbimPoint3D> ps, int expected)
        {
            var r = new Regex("\nVe\n"); // regex to count the vertices in the brep (unix style endline)
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            using (var txn = m.BeginTransaction(""))
            {
                var poly = m.Instances.New<IfcPolyline>();
                foreach (var p in ps)
                {
                    var cp = m.Instances.New<IfcCartesianPoint>();
                    cp.SetXY(p.X, p.Y);
                    poly.Points.Add(cp);
                }
                var area = m.Instances.New<IfcArbitraryClosedProfileDef>();
                area.OuterCurve = poly;
                var solid = geomEngine.CreateFace(area, logger);
                var pline = geomEngine.CreateCurve(poly, logger);
                var t = solid.ToBRep;
                Debug.WriteLine(t);
                Assert.AreEqual(4, r.Matches(t).Count);
                return t;
            }
        }
	}
}
