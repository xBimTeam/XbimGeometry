using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;

namespace GeometryTests
{
	[TestClass]
	public class ShapeInstanceEnumerationTests
	{
		[TestMethod]
		public void CorrectShapesWithOpenings()
		{
			using (var m = IfcStore.Open(@"SolidTestFiles\WallWithOpening.ifc")) {
				var context = new Xbim3DModelContext(m);
				context.CreateContext();

				var shapes = context.ShapeInstances().ToArray();
				Assert.AreEqual(3, shapes.Length, "Number of geometry instances");

				var opsAndAdsIncludedShape = shapes.FirstOrDefault(s => s.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded);
				Assert.IsNotNull(opsAndAdsIncludedShape, "Finished wall shape, that has all necessary openings and projections incorporated");

				var opsAndAdsOnlyShape = shapes.FirstOrDefault(s => s.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsOnly);
				Assert.IsNotNull(opsAndAdsOnlyShape, "Opening shape.");

				var opsAndAdsExcludedShape = shapes.FirstOrDefault(s => s.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded);
				Assert.IsNotNull(opsAndAdsExcludedShape, "Wall shape, that was not cut with the opening shape.");
			}
		}

		[TestMethod]
		public void CorrectShapesWithProjections()
		{
			using (var m = IfcStore.Open(@"SolidTestFiles\WallWithConvex.ifc")) {
				var context = new Xbim3DModelContext(m);
				context.CreateContext();

				var shapes = context.ShapeInstances().ToArray();
				Assert.AreEqual(3, shapes.Length, "Number of geometry instances");

				var opsAndAdsIncludedShape = shapes.FirstOrDefault(s => s.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded);
				Assert.IsNotNull(opsAndAdsIncludedShape, "Finished wall shape, that has all necessary openings and projections incorporated");

				var opsAndAdsOnlyShape = shapes.FirstOrDefault(s => s.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsOnly);
				Assert.IsNotNull(opsAndAdsOnlyShape, "Projection shape.");

				var opsAndAdsExcludedShape = shapes.FirstOrDefault(s => s.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded);
				Assert.IsNotNull(opsAndAdsExcludedShape, "Wall shape, that was not projected by the projection shape.");
			}
		}

	}
}
