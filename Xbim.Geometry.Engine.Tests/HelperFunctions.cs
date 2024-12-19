﻿using Microsoft.Extensions.Logging;

using System;
using System.Diagnostics;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;
using Xunit;
using FluentAssertions;
namespace Xbim.Geometry.Engine.Tests
{
    public static class HelperFunctions
    {
        public static void GeneralTest(IXbimSolid solid, bool ignoreVolume = false, bool isHalfSpace = false, int entityLabel = 0)
        {
            if (ignoreVolume && !isHalfSpace && solid.Volume == 0)
            {
                Trace.WriteLine(String.Format("Entity  #{0} has zero volume>", entityLabel));
            }
            if (!ignoreVolume)
                solid.Volume.Should().BeGreaterThan(0, "Volume should be greater than 0");
            solid.SurfaceArea.Should().BeGreaterThan(0, "Surface Area should be greater than 0");
            solid.IsValid.Should().BeTrue();

            if (!isHalfSpace)
            {
                foreach (var face in solid.Faces)
                {
                    face.OuterBound.IsValid.Should().BeTrue("Face has no outer bound in #" + entityLabel);
                    face.Area.Should().BeGreaterThan(0, "Face area should be greater than 0 in #" + entityLabel);
                    face.Perimeter.Should().BeGreaterThan(0, "Face perimeter should be breater than 0 in #" + entityLabel);

                    if (face.IsPlanar)
                    {
                        face.Normal.IsInvalid().Should().BeFalse("Face normal is invalid in #" + entityLabel);
                        //  face.OuterBound.Edges.Count>2, "A face should have at least 3 edges");
                        //   !face.OuterBound.Normal.IsInvalid(), "Face outerbound normal is invalid in #" + entityLabel);
                        //   face.OuterBound.IsPlanar, "Face is planar but wire is not in #" + entityLabel);
                    }
                    else
                    {
                        face.OuterBound.IsPlanar.Should().BeFalse("Face is not planar but wire is planar in #" + entityLabel);
                    }

                    foreach (var edge in face.OuterBound.Edges)
                    {
                        edge.EdgeGeometry.IsValid.Should().BeTrue("Edge element is invalid in #" + entityLabel);
                        edge.EdgeStart.IsValid.Should().BeTrue("Edge start is invalid in #" + entityLabel);
                        edge.EdgeEnd.IsValid.Should().BeTrue("Edge end is invalid in #" + entityLabel);
                    }
                }
            }
        }

        //public static double ConvertGeometryAllCompositesAtOnce(IXbimGeometryEngine geometryEngine, ShapeGeometryDTO geomDto,  ILogger logger =null)
        //{
        //    var geomHelper = ShapeGeometryHelper.BuildFromDTO(geomDto);
        //    var assetModel = new AssetModel();
        //    var shapeGeometry = new AimShapeGeometry(assetModel)
        //    {
        //        AssetModelId = 1,
        //        ShapeGeometryId = geomDto.GeometryId,
        //        EntityVariantId = geomDto.EntityVariantId,
        //        MaterialId = geomDto.ShapeMaterialId,
        //        Volume = 0,
        //    };

        //    var sw = new Stopwatch();
        //    var model = geomHelper.Geometry.Model;
        //    if (geomHelper.Voids != null && geomHelper.Voids.Any(v => v != null))
        //    {
        //        var cuts = geometryEngine.CreateSolidSet();
        //        foreach (var voidItem in geomHelper.Voids)
        //        {
        //            var cut = geometryEngine.CreateGeometryObjectSet();
        //            foreach (var shapeRep in voidItem.Flatten())
        //            {
        //                var geometry = geometryEngine.Create(shapeRep.GeometryItem, logger);
        //                if (geometry != null && geometry.IsValid)
        //                {
        //                    if (shapeRep.Transform.HasValue)
        //                    {
        //                        var transformed = geometry.Transform(shapeRep.Transform.Value);
        //                        cut.Add(transformed);
        //                    }
        //                    else
        //                        cut.Add(geometry);
        //                }
        //            }
        //            cuts.Add(cut);
        //        }

        //        //    using (var  = GeometryEngine.Create(_geometryRepresentationItem))
        //        var bodyGeom = geomHelper.Geometry.CreateSolids(geometryEngine, logger);
        //        {
        //            if (bodyGeom != null && bodyGeom.Any())
        //            {
        //                var precision = model.ModelFactors.Precision;
        //                var deflection = model.ModelFactors.DeflectionTolerance;
        //                var deflectionAngle = model.ModelFactors.DeflectionAngle;


        //                try
        //                {

        //                    sw.Start();
        //                    using (var bodyCut = bodyGeom.Cut(cuts, precision))
        //                    {

        //                    }
        //                }
        //                catch (Exception e)
        //                {
        //                    throw new Exception($"Exception cutting voids in Geometric Shape #{geomHelper.ShapeGeometryId}, Entity #{geomHelper.EntityVariantId}, Error[{e.Message}]");
        //                }
        //                sw.Stop();
        //            }
        //        }
        //        //cleanup up the 
        //        foreach (var cut in cuts) cut.Dispose();
        //    }
        //    else
        //    {
        //        var tessellator = new XbimTessellator(model, XbimGeometryType.PolyhedronBinary);
        //        if (tessellator.CanMesh(geomHelper.Geometry))
        //        {
        //            IXbimShapeGeometryData shapeData = tessellator.Mesh(geomHelper.Geometry);
        //            shapeGeometry.BoundingBox = shapeData.BoundingBox;
        //            shapeGeometry.Triangulation = shapeData.ShapeData;
        //        }
        //        else
        //        {
        //            using (var geometry = geometryEngine.Create(geomHelper.Geometry, logger))
        //            {
        //                if (geometry != null)
        //                {
        //                    IXbimShapeGeometryData shapeData = geometryEngine.CreateShapeGeometry(geometry, model.ModelFactors.Precision, model.ModelFactors.DeflectionTolerance,
        //                        model.ModelFactors.DeflectionAngle, XbimGeometryType.PolyhedronBinary, logger);
        //                    shapeGeometry.BoundingBox = shapeData.BoundingBox;
        //                    shapeGeometry.Triangulation = shapeData.ShapeData;
        //                    var solid = geometry as IXbimSolid;
        //                    shapeGeometry.Volume = solid?.Volume ?? 0;
        //                }
        //            }
        //        }
        //    }
        //    sw.Stop();           
        //    return sw.ElapsedMilliseconds;
        //}

        public static void IsValidSolid(IXbimSolid solid, bool ignoreVolume = false, bool isHalfSpace = false, int entityLabel = 0)
        {
            // ReSharper disable once CompareOfFloatsByEqualityOperator
            if (ignoreVolume && !isHalfSpace && solid.Volume == 0)
            {
                Trace.WriteLine(String.Format("Entity  #{0} has zero volume>", entityLabel));
            }
            solid.Should().NotBeNull();
            if (!ignoreVolume) solid.Volume.Should().BeGreaterThan(0);
            solid.SurfaceArea.Should().BeGreaterThan(0);
            solid.IsValid.Should().BeTrue();

            if (!isHalfSpace)
            {
                foreach (var face in solid.Faces)
                {

                    face.OuterBound.IsValid.Should().BeTrue();
                   
                    face.Perimeter.Should().BeGreaterThan(0);
                    //srl this is not a valid test with spline surfaces
                    //if (face.IsPlanar)
                    //{
                    //    !face.Normal.IsInvalid(), "Face normal is invalid in #" + entityLabel);
                    //    //  face.OuterBound.Edges.Count>2, "A face should have at least 3 edges");
                    //    //   !face.OuterBound.Normal.IsInvalid(), "Face outerbound normal is invalid in #" + entityLabel);
                    //    //   face.OuterBound.IsPlanar, "Face is planar but wire is not in #" + entityLabel);
                    //}
                    //else
                    //    Assert.IsFalse(face.OuterBound.IsPlanar, "Face is not planar but wire is planar in #" + entityLabel);
                    foreach (var edge in face.OuterBound.Edges)
                    {
                        edge.EdgeGeometry.IsValid.Should().BeTrue();
                        edge.EdgeStart.IsValid.Should().BeTrue();
                        edge.EdgeEnd.IsValid.Should().BeTrue();
                    }
                }
            }
        }

        private static IXbimSolidSet? CreateSolids(this IIfcGeometricRepresentationItem repItem, IXbimGeometryEngine engine, ILogger logger)
        {
            var sSet = engine.CreateSolidSet();
            var sweptArea = repItem as IIfcSweptAreaSolid;
            if (sweptArea != null && (sweptArea.SweptArea is IIfcCompositeProfileDef))
            {
                //this is necessary as composite profile definitions produce multiple solid            
                return engine.CreateSolidSet(sweptArea, logger);
            }
            else if( repItem is IIfcSweptAreaSolid)
            {
                sSet.Add(engine.CreateSolid(repItem as IIfcSweptAreaSolid, logger));
                return sSet;
            }
            else if (repItem is IIfcSweptDiskSolid)
            {
                sSet.Add(engine.CreateSolid(repItem as IIfcSweptDiskSolid, logger));
                return sSet;
            }
            else if (repItem is IIfcBooleanResult) { return engine.CreateSolidSet(repItem as IIfcBooleanResult, logger); }
            else if (repItem is IIfcBooleanClippingResult) { return engine.CreateSolidSet(repItem as IIfcBooleanClippingResult, logger); }
            else if (repItem is IIfcCsgSolid) { return engine.CreateSolidSet(repItem as IIfcCsgSolid, logger); }
            else if (repItem is IIfcFacetedBrep) return engine.CreateSolidSet(repItem as IIfcFacetedBrep, logger);
            else if (repItem is IIfcTriangulatedFaceSet) return engine.CreateSolidSet(repItem as IIfcTriangulatedFaceSet);
            else if (repItem is IIfcShellBasedSurfaceModel) return engine.CreateSolidSet(repItem as IIfcShellBasedSurfaceModel);
            else if (repItem is IIfcFaceBasedSurfaceModel) return engine.CreateSolidSet(repItem as IIfcFaceBasedSurfaceModel);
            else if (repItem is IIfcBoundingBox) { sSet.Add(engine.CreateSolid(repItem as IIfcBoundingBox, logger)); return sSet; }
            else if (repItem is IIfcSectionedSpine) { sSet.Add(engine.CreateSolid(repItem as IIfcSectionedSpine, logger)); return sSet; }
            else if (repItem is IIfcCsgPrimitive3D) { sSet.Add(engine.CreateSolid(repItem as IIfcCsgPrimitive3D, logger)); return sSet; }
            else if (repItem is IIfcAdvancedBrep) { sSet.Add(engine.CreateSolid(repItem as IIfcAdvancedBrep, logger)); return sSet; }
            else return null;

            
        }
    }
}
