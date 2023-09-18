using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Linq;
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
    }
}
