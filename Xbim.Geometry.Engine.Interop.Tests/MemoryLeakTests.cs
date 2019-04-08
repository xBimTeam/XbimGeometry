using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class MemoryLeakTests
    {
        static private IXbimGeometryEngine geomEngine;
        

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            geomEngine = new XbimGeometryEngine();
        }
        [ClassCleanup]
        static public void Cleanup()
        {
           
            geomEngine = null;
           
        }
        [TestMethod]
        public void simple_vertex_is_constructed_and_disposed()
        {
            //IXbimVertex vertex;
            {
                var aPoint = new Common.Geometry.XbimPoint3D(1, 2, 3);
                //using (var pt = geomEngine.CreateVertexPoint(aPoint, 0.005))
                //{
                //    vertex = pt;
                //    Assert.AreEqual(pt.VertexGeometry.X, aPoint.X);
                //    Assert.AreEqual(pt.VertexGeometry.Y, aPoint.Y);
                //    Assert.AreEqual(pt.VertexGeometry.Z, aPoint.Z);
                //}
                //Assert.IsFalse(vertex.IsValid);
                var vertices = new List<IXbimVertex>(10000);
                for (int i = 0; i < 1000000; i++)
                {
                    vertices.Add(geomEngine.CreateVertexPoint(aPoint, 0.005));
                }
                vertices = null;
                GC.Collect();
            }
        }

    }
}
