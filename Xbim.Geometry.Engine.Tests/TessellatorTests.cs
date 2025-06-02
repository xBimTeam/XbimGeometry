using FluentAssertions;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;
using Xbim.Common.XbimExtensions;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.Tessellator;
using Xunit;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    public class TessellatorTests
    {
        [Fact]
        public void Test_Large_Coordinates_Reduction()
        {
            XbimGeometryType tp = XbimGeometryType.PolyhedronBinary;
            using var model = IfcStore.Open("TestFiles\\LargeTriangulatedCoordinates.ifc");
            var xbimTessellator = new XbimTessellator(model, tp);
            var representation = model.Instances.FirstOrDefault<IIfcFacetedBrep>();
            var shape = xbimTessellator.Mesh(representation);

            // geometry should have a local displacement
            shape.LocalShapeDisplacement.HasValue.Should().BeTrue();

            // it should be more than 6 200 000
            shape.Should().NotBeNull();
            var displacement = shape.LocalShapeDisplacement;
            displacement.Should().NotBeNull();
            displacement.HasValue.Should().BeTrue();
            var distance = displacement.Value.Length;
            distance.Should().BeGreaterThan(6200000);

            var ms = new MemoryStream(((IXbimShapeGeometryData)shape).ShapeData);
            var br = new BinaryReader(ms);
            var geometry = br.ReadShapeTriangulation();

            // vertex geometry should be small
            var vertex = geometry.Vertices.First();

            vertex.X.Should().BeLessThan(1000);
            vertex.Y.Should().BeLessThan(1000);
            vertex.Z.Should().BeLessThan(1000);

            // bounding box should be at [0,0,0] position
            var bb = shape.BoundingBox;
            var pos = bb.Location;
            var test = Math.Abs(pos.X + pos.Y + pos.Z);
            test.Should().BeLessThan(0.1);
        }

        [Fact]
        public void Test_PolygonalFaceSet_Tessellation()
        {
            XbimGeometryType tp = XbimGeometryType.PolyhedronBinary;
            using var model = IfcStore.Open("TestFiles\\Ifc4TestFiles\\polygonal-face-tessellation.ifc");
            var xbimTessellator = new XbimTessellator(model, tp);
            XbimShapeGeometry? shapeGeom = null;
            var shape = model.Instances.FirstOrDefault<IIfcPolygonalFaceSet>();
            shapeGeom = xbimTessellator.Mesh(shape);
            shapeGeom.BoundingBox.Volume.Should().Be(8000000000000);
        }

        [Fact]
        public void TestBoundingBoxSize()
        {
            XbimGeometryType tp = XbimGeometryType.PolyhedronBinary;
            using var model = IfcStore.Open("TestFiles\\Roof-01_BCAD.ifc");
            var xbimTessellator = new XbimTessellator(model, tp);
            XbimShapeGeometry? shapeGeom = null;
            var shape = model.Instances[1192] as IIfcGeometricRepresentationItem;
            shapeGeom = xbimTessellator.Mesh(shape);
            Debug.WriteLine(shapeGeom.BoundingBox);
        }

        [Fact]
        public void TestPnSize_Add2_Support()
        {
            XbimGeometryType tp = XbimGeometryType.PolyhedronBinary;
            using var model = IfcStore.Open("TestFiles\\IfcTriangulatedFaceSet.ifc");
            var xbimTessellator = new XbimTessellator(model, tp);
            XbimShapeGeometry? shapeGeom = null;
            var shape = model.Instances[48] as IIfcGeometricRepresentationItem;
            shapeGeom = xbimTessellator.Mesh(shape);
            Debug.WriteLine(shapeGeom.BoundingBox);
        }
    }
}
