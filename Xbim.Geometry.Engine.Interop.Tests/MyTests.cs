using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Linq;
using FluentAssertions;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;
using System.Diagnostics;


namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class MyTests
    {
        [TestMethod]
        public void Ifc4_MyTest_ARCHI_COMBINED()
        {
            using (var m = IfcStore.Open(@"C:\LT\Dev\IFC_Issues\ARCHI COMBINED.ifc"))
            {
                m.Instances.Count.Should().Be(150991);

                var context = new Xbim3DModelContext(m);
                context.CreateContext();

                var numShapes = context.ShapeGeometries().ToArray().Length;
                numShapes.Should().NotBe(0);
            }
        }

        [TestMethod]
        public void Ifc4_MyTest_Architectural()
        {
            using (var m = IfcStore.Open(@"C:\LT\Dev\IFC_Issues\Architectural model (1).ifc"))
            {
                m.Instances.Count.Should().Be(167626);

                var context = new Xbim3DModelContext(m);
                context.CreateContext();

                var numShapes = context.ShapeGeometries().ToArray().Length;
                numShapes.Should().NotBe(0);
            }
        }

        [TestMethod]
        public void Ifc2x3_MyTest_1_Gridline()
        {
            using (var m = IfcStore.Open(@"C:\LT\Dev\IFC_Issues\1_Gridline test model.ifc"))
            {
                m.Instances.Count.Should().Be(28315);

                var context = new Xbim3DModelContext(m);
                context.CreateContext();

                var numShapes = context.ShapeGeometries().ToArray().Length;
                numShapes.Should().NotBe(0);
            }
        }

        [TestMethod]
        public void Ifc2x3_MyTest_2_Gridline()
        {
            using (var m = IfcStore.Open(@"C:\LT\Dev\IFC_Issues\2_Gridline test model.ifc"))
            {
                m.Instances.Count.Should().Be(28316);

                var context = new Xbim3DModelContext(m);
                context.CreateContext();

                var numShapes = context.ShapeGeometries().ToArray().Length;
                numShapes.Should().NotBe(0);
            }
        }

        [TestMethod]
        public void Ifc2x3_MyTest_3_Gridline()
        {
            using (var m = IfcStore.Open(@"C:\LT\Dev\IFC_Issues\3_Gridline test model.ifc"))
            {
                m.Instances.Count.Should().Be(28315);

                var context = new Xbim3DModelContext(m);
                context.CreateContext();

                var numShapes = context.ShapeGeometries().ToArray().Length;
                numShapes.Should().NotBe(0);
            }
        }

        [TestMethod]
        public void Ifc_MyTest_IssueR23()
        {
            using (var m = IfcStore.Open(@"C:\LT\Dev\IFC_Issues\B1_AR_R23.ifc"))
            {
                var context = new Xbim3DModelContext(m);
                context.CreateContext();

                var numShapes = context.ShapeGeometries().ToArray().Length;
                Console.WriteLine("numShapes: " + numShapes);
                numShapes.Should().NotBe(0);
            }
        }

        [TestMethod]
        public void Ifc_MyTest_IssueBLK35B()
        {
            using (var m = IfcStore.Open(@"C:\LT\Dev\IFC_Issues\CS_BLK35B_CCKN1C1718_-.ifc"))
            {
                var context = new Xbim3DModelContext(m);
                context.CreateContext();

                var numShapes = context.ShapeGeometries().ToArray().Length;
                Console.WriteLine("numShapes: " + numShapes);
                numShapes.Should().NotBe(0);
            }
        }

        [TestMethod]
        public void Ifc4_MyTest_DegeneratedPoint()
        {
            var stopwatch = new Stopwatch();

            Console.WriteLine("Testing Small");
            Console.WriteLine("- Trying to open...");

            stopwatch.Start();
            using (var m = IfcStore.Open(@"C:\LT\Dev\IFC_Issues\rua_degenerada_na_origem.ifc"))
            {
                Console.WriteLine("  done (" + stopwatch.Elapsed + "ms).");

                m.Instances.Count.Should().NotBe(0);

                Console.WriteLine(" Instances found: " + m.Instances.Count);
                Console.WriteLine(" - Creating context...");
                stopwatch.Restart();

                var context = new Xbim3DModelContext(m);
                context.CreateContext();

                Console.WriteLine("  done (" + stopwatch.Elapsed + "ms).");
                stopwatch.Restart();

                var shapes = context.ShapeGeometries().ToArray();
                Console.WriteLine("numShapes: " + shapes.Length);

                foreach (var shape in shapes)
                {
                    var vertices = shape.Vertices.ToArray();

                    for (int i = 0; i < vertices.Length; ++i)
                    {
                        if (vertices[i].X.Equals(0.0))
                        {
                            Console.WriteLine("shape.Format: " + shape.Format);
                            Console.WriteLine("Vertices: " + vertices.Length);

                            Console.WriteLine("vertex[" + i + "]: " + vertices[i]);
                            Console.WriteLine("ZERO");
                        }
                    }

                    var faces = shape.Faces.ToArray();

                    foreach (var face in faces)
                    {
                        var v0 = vertices[face.Indices.ElementAt(0)];
                        var v1 = vertices[face.Indices.ElementAt(1)];
                        var v2 = vertices[face.Indices.ElementAt(2)];

                        if (v0.X.Equals(0.0)
                            || v1.X.Equals(0.0)
                            || v2.X.Equals(0.0))
                        {
                            Console.WriteLine("  - FACE WITH (0,0,0): ");
                        }
                    }
                }

                var numShapes = context.ShapeGeometries().ToArray().Length;


                Console.WriteLine(" Shapes found: " + numShapes);
                numShapes.Should().NotBe(0);
                Console.WriteLine("  done (" + stopwatch.Elapsed + "ms).");
            }
        }

        [TestMethod]
        public void Ifc4_MyTest_Rebars()
        {
            var stopwatch = new Stopwatch();

            Console.WriteLine("Testing Small");
            Console.WriteLine("- Trying to open...");

            stopwatch.Start();
            using (var m = IfcStore.Open(@"C:\LT\Dev\IFC_Issues\RebarsOneElement.ifc"))
            {
                Console.WriteLine("  done (" + stopwatch.Elapsed + "ms).");

                m.Instances.Count.Should().NotBe(0);

                Console.WriteLine(" Instances found: " + m.Instances.Count);
                Console.WriteLine(" - Creating context...");
                stopwatch.Restart();

                var context = new Xbim3DModelContext(m);
                context.CreateContext();

                Console.WriteLine("  done (" + stopwatch.Elapsed + "ms).");
                stopwatch.Restart();

                var shapes = context.ShapeGeometries().ToArray();
                Console.WriteLine("numShapes: " + shapes.Length);

                foreach (var shape in shapes)
                {
                    var vertices = shape.Vertices.ToArray();

                    for (int i = 0; i < vertices.Length; ++i)
                    {
                        if (vertices[i].X.Equals(0.0))
                        {
                            Console.WriteLine("shape.Format: " + shape.Format);
                            Console.WriteLine("Vertices: " + vertices.Length);

                            Console.WriteLine("vertex[" + i + "]: " + vertices[i]);
                            Console.WriteLine("ZERO");
                        }
                    }

                    var faces = shape.Faces.ToArray();

                    foreach (var face in faces)
                    {
                        var v0 = vertices[face.Indices.ElementAt(0)];
                        var v1 = vertices[face.Indices.ElementAt(1)];
                        var v2 = vertices[face.Indices.ElementAt(2)];

                        if (v0.X.Equals(0.0)
                            || v1.X.Equals(0.0)
                            || v2.X.Equals(0.0))
                        {
                            Console.WriteLine("  - FACE WITH (0,0,0): ");
                        }
                    }
                }

                var numShapes = context.ShapeGeometries().ToArray().Length;


                Console.WriteLine(" Shapes found: " + numShapes);
                numShapes.Should().NotBe(0);
                Console.WriteLine("  done (" + stopwatch.Elapsed + "ms).");
            }
        }
    }
}
