using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text.RegularExpressions;
using Xbim.Common.Geometry;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.ProfileResource;
using Xbim.IO.Memory;
using Xunit;
namespace Xbim.Geometry.Engine.Interop.Tests
{
    
	public class ShapeAdjustementTests
	{
        

        static private ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        static private ILogger logger = loggerFactory.CreateLogger<ShapeAdjustementTests>();
        

        [Fact]
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
            returnedbrep.Should().NotContain(".000001", "because it removed the wrong point");
        }

  //      [TestMethod, Ignore("This is a bad concept, we should not remove line segments that are colinear(aligned) when building a face. It is perfectly valid for two faces to abut two colinear segments of another face")]
		//public void PolygonAlignedCleanup()
		//{
  //          /*
		//	 * From PR 301
		//	 * 
		//	#172= IFCCARTESIANPOINT((3062.13203435597,-150.));
		//	#174= IFCCARTESIANPOINT((3362.13203435597,150.));
		//	#176= IFCCARTESIANPOINT((2937.86796564404,150.));
		//	#178= IFCCARTESIANPOINT((0.,150.));
		//	#180= IFCCARTESIANPOINT((0.,-150.));
		//	#182= IFCPOLYLINE((#172,#174,#176,#178,#180,#172));
		//	#184= IFCARBITRARYCLOSEDPROFILEDEF(.AREA.,$,#182);
		//	#187= IFCAXIS2PLACEMENT3D(#6,$,$);
		//	#188= IFCEXTRUDEDAREASOLID(#184,#187,#20,3000.); 
		//	 */

  //          List<XbimPoint3D> ps = new List<XbimPoint3D>();
  //          ps.Add(new XbimPoint3D(3062.13203435597,-150,0));
  //          ps.Add(new XbimPoint3D(3362.13203435597,150,0));
  //          ps.Add(new XbimPoint3D(2937.86796564404,150,0)); // this point is aligned with the surrounding ones, should be removed
  //          ps.Add(new XbimPoint3D(0,150,0));
  //          ps.Add(new XbimPoint3D(0,-150,0));
  //          Check(ps, 4);



  //          ps = new List<XbimPoint3D>();
  //          ps.Add(new XbimPoint3D(0, 0, 0));
  //          ps.Add(new XbimPoint3D(10, 0, 0));
  //          ps.Add(new XbimPoint3D(10, 10, 0)); 
  //          ps.Add(new XbimPoint3D(5.000001, 10, 0)); // this should be removed because aligned
  //          ps.Add(new XbimPoint3D(0, 10, 0));
  //          var returnedbrep = Check(ps, 4);
  //          Assert.IsFalse(returnedbrep.Contains(".000001"), "because it removed the wrong point"); // should not find the point with .00001 coordinate

  //          // more complex case with overlapping wires
  //          ps = new List<XbimPoint3D>();
  //          ps.Add(new XbimPoint3D(0, 0, 0));
  //          ps.Add(new XbimPoint3D(10, 0, 0));
  //          ps.Add(new XbimPoint3D(10, 10, 0));
  //          ps.Add(new XbimPoint3D(3.000001, 10, 0)); // this should be removed because aligned
  //          ps.Add(new XbimPoint3D(7.000001, 10, 0)); // this goes in the opposite direction of the previous segment
  //          ps.Add(new XbimPoint3D(0, 10, 0));
  //          returnedbrep = Check(ps, 4);
  //          Assert.IsFalse(returnedbrep.Contains(".000001"), "because it removed the wrong point"); // should not find the point with .00001 coordinate

  //      }

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
                var geomEngine = new XbimGeometryEngine(m, loggerFactory);
                var solid = geomEngine.CreateFace(area, logger);
                var pline = geomEngine.CreateCurve(poly, logger);
                var t = solid.ToBRep;
                Debug.WriteLine(t);
                r.Matches(t).Count.Should().Be(4);
                return t;
            }
        }
	}
}
