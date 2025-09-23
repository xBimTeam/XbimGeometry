using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.Common.Geometry;
using Xbim.Common.XbimExtensions;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.Tessellator;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{
    public class TessellatorTests
    {
        [Fact]
        public void Test_Large_Coordinates_Reduction()
        {
            XbimGeometryType tp = Xbim.Common.Geometry.XbimGeometryType.PolyhedronBinary;
            using (var model = IfcStore.Open("TestFiles\\LargeTriangulatedCoordinates.ifc"))
            {
                var xbimTessellator = new XbimTessellator(model, tp);
                var representation = model.Instances.FirstOrDefault<IIfcFacetedBrep>();
                var shape = xbimTessellator.Mesh(representation);

                // geometry should have a local displacement
                Assert.True(shape.LocalShapeDisplacement.HasValue);

                // it should be more than 6 200 000
                var distance = shape.LocalShapeDisplacement.Value.Length;
                Assert.True(distance > 6200000);

                var ms = new MemoryStream(((IXbimShapeGeometryData)shape).ShapeData);
                var br = new BinaryReader(ms);
                var geometry = br.ReadShapeTriangulation();

                // vertex geometry should be small
                var vertex = geometry.Vertices.First();
                Assert.True(vertex.X < 1000);
                Assert.True(vertex.Y < 1000);
                Assert.True(vertex.Z < 1000);

                // bounding box should be at [0,0,0] position
                var bb = shape.BoundingBox;
                var pos = bb.Location;
                var test = Math.Abs(pos.X + pos.Y + pos.Z);
                Assert.True(test < 0.1)
;
            }
        }

        [Fact]
        public void Test_PolygonalFaceSet_Tessellation()
        {
            XbimGeometryType tp = Xbim.Common.Geometry.XbimGeometryType.PolyhedronBinary;
            using (var model = IfcStore.Open("TestFiles\\Ifc4TestFiles\\polygonal-face-tessellation.ifc"))
            {
                var xbimTessellator = new XbimTessellator(model, tp);
                XbimShapeGeometry shapeGeom;

                var shape = model.Instances.FirstOrDefault<IIfcPolygonalFaceSet>();
                shapeGeom = xbimTessellator.Mesh(shape);
                Assert.Equal(8000000000000, shapeGeom.BoundingBox.Volume);
            }
        }

        [Fact]
        public void TestBoundingBoxSize()
        {
            XbimGeometryType tp = Xbim.Common.Geometry.XbimGeometryType.PolyhedronBinary;
            using (var model = IfcStore.Open("TestFiles\\IfcExamples\\Roof-01_BCAD.ifc"))
            {
                var xbimTessellator = new XbimTessellator(model, tp);
                XbimShapeGeometry shapeGeom;

                var shape = model.Instances[1192] as IIfcGeometricRepresentationItem;
                shapeGeom = xbimTessellator.Mesh(shape);
                Debug.WriteLine(shapeGeom.BoundingBox);
            }
        }

        [Fact]
        public void TestPnSize_Add2_Support()
        {
            XbimGeometryType tp = Xbim.Common.Geometry.XbimGeometryType.PolyhedronBinary;
            using (var model = IfcStore.Open("TestFiles\\Ifc4TestFiles\\IfcTriangulatedFaceSet.ifc"))
            {
                var xbimTessellator = new XbimTessellator(model, tp);
                XbimShapeGeometry shapeGeom;

                var shape = model.Instances[48] as IIfcGeometricRepresentationItem;
                shapeGeom = xbimTessellator.Mesh(shape);
                Debug.WriteLine(shapeGeom.BoundingBox);
            }
        }
    }
}
