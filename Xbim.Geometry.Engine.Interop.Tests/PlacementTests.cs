using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class PlacementTests
    {
        static private IXbimGeometryEngine geomEngine;
        static private ILoggerFactory loggerFactory;
        static private ILogger logger;

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            loggerFactory = new LoggerFactory().AddDebug(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<IfcBooleanTests>();
        }

        [TestMethod]
        public void horizontal_alignment_without_transition_curve_test()
        {
            using (var er = new EntityRepository<IIfcAlignment2DHorizontal>("horizontal-alignment-without-transition-curve", meter: 1, precision: 1e-3, inRadians: true))
            {
                var alignmentFace = geomEngine.CreateFace(er.Entity, logger);
                Assert.AreEqual(706.85975907255215, alignmentFace.Area, 1e-3);
                Assert.AreEqual(3, alignmentFace.OuterBound.Edges.Count);
                Assert.AreEqual(3, alignmentFace.OuterBound.Vertices.Count);
            }
        }
        [TestMethod]
        public void can_build_horizontal_alignment_solid_shape_test()
        {
            using (var er = new EntityRepository<IIfcAlignment>("alignment-without-vertical_component", meter: 1, precision: 1e-3, inRadians: true))
            {
                var alignment = geomEngine.CreateAlignment(er.Entity, logger);
                Assert.IsTrue(alignment.IsValid);
            }
        }

    }
}
