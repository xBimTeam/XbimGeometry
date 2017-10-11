using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Aim.Edm;
using Xbim.Aim.Helpers;
using Xbim.Common.Geometry;
using Xbim.Geometry.ProtoBuffer;
using Xbim.Ifc4.Interfaces;
using Xbim.Tessellator;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    public static class HelperFunctions
    {

        public static double ConvertGeometryAllCompositesAtOnce(IXbimGeometryEngine geometryEngine, ShapeGeometryDTO geomDto,  ILogger logger =null)
        {
            var geomHelper = ShapeGeometryHelper.BuildFromDTO(geomDto);
            var assetModel = new AssetModel();
            var shapeGeometry = new AimShapeGeometry(assetModel)
            {
                AssetModelId = 1,
                ShapeGeometryId = geomDto.GeometryId,
                EntityVariantId = geomDto.EntityVariantId,
                MaterialId = geomDto.ShapeMaterialId,
                Volume = 0,
            };

            var sw = new Stopwatch();
            var model = geomHelper.Geometry.Model;
            if (geomHelper.Voids != null && geomHelper.Voids.Any(v => v != null))
            {
                var cuts = geometryEngine.CreateSolidSet();
                foreach (var voidItem in geomHelper.Voids)
                {
                    var cut = geometryEngine.CreateGeometryObjectSet();
                    foreach (var shapeRep in voidItem.Flatten())
                    {
                        var geometry = geometryEngine.Create(shapeRep.GeometryItem, logger);
                        if (geometry != null && geometry.IsValid)
                        {
                            if (shapeRep.Transform.HasValue)
                            {
                                var transformed = geometry.Transform(shapeRep.Transform.Value);
                                cut.Add(transformed);
                            }
                            else
                                cut.Add(geometry);
                        }
                    }
                    cuts.Add(cut);
                }
                
                //    using (var  = GeometryEngine.Create(_geometryRepresentationItem))
                var bodyGeom = geomHelper.Geometry.CreateSolids(geometryEngine, logger);
                {
                    if (bodyGeom != null && bodyGeom.Any())
                    {
                        var precision = model.ModelFactors.Precision;
                        var deflection = model.ModelFactors.DeflectionTolerance;
                        var deflectionAngle = model.ModelFactors.DeflectionAngle;
                        
                        
                        try
                        {
                            
                            sw.Start();
                            using (var bodyCut = bodyGeom.Cut(cuts, precision))
                            {
                               
                            }
                        }
                        catch (Exception e)
                        {
                            throw new Exception($"Exception cutting voids in Geometric Shape #{geomHelper.ShapeGeometryId}, Entity #{geomHelper.EntityVariantId}, Error[{e.Message}]");
                        }
                        sw.Stop();
                    }
                }
                //cleanup up the 
                foreach (var cut in cuts) cut.Dispose();
            }
            else
            {
                var tessellator = new XbimTessellator(model, XbimGeometryType.PolyhedronBinary);
                if (tessellator.CanMesh(geomHelper.Geometry))
                {
                    IXbimShapeGeometryData shapeData = tessellator.Mesh(geomHelper.Geometry);
                    shapeGeometry.BoundingBox = shapeData.BoundingBox;
                    shapeGeometry.Triangulation = shapeData.ShapeData;
                }
                else
                {
                    using (var geometry = geometryEngine.Create(geomHelper.Geometry, logger))
                    {
                        if (geometry != null)
                        {
                            IXbimShapeGeometryData shapeData = geometryEngine.CreateShapeGeometry(geometry, model.ModelFactors.Precision, model.ModelFactors.DeflectionTolerance,
                                model.ModelFactors.DeflectionAngle, XbimGeometryType.PolyhedronBinary, logger);
                            shapeGeometry.BoundingBox = shapeData.BoundingBox;
                            shapeGeometry.Triangulation = shapeData.ShapeData;
                            var solid = geometry as IXbimSolid;
                            shapeGeometry.Volume = solid?.Volume ?? 0;
                        }
                    }
                }
            }
            sw.Stop();           
            return sw.ElapsedMilliseconds;
        }
        
        private static IXbimSolidSet CreateSolids(this IIfcGeometricRepresentationItem repItem, IXbimGeometryEngine engine, ILogger logger)
        {
            var sSet = engine.CreateSolidSet();

            if (repItem is IIfcSolidModel)
            {
                var sweptArea = repItem as IIfcSweptAreaSolid;
                //this is necessary as composite profile definitions produce multiple solids
                if (sweptArea != null && sweptArea.SweptArea is IIfcCompositeProfileDef)
                    return engine.CreateSolidSet(sweptArea, logger);
                else
                    sSet.Add(engine.CreateSolid(repItem as IIfcSolidModel, logger)); return sSet;
            }
            else if (repItem is IIfcBooleanResult) { sSet.Add(engine.CreateSolid(repItem as IIfcBooleanResult, logger)); return sSet; }
            else if (repItem is IIfcFacetedBrep) return engine.CreateSolidSet(repItem as IIfcFacetedBrep, logger);
            else if (repItem is IIfcTriangulatedFaceSet) return engine.CreateSolidSet(repItem as IIfcTriangulatedFaceSet);
            else if (repItem is IIfcShellBasedSurfaceModel) return engine.CreateSolidSet(repItem as IIfcShellBasedSurfaceModel);
            else if (repItem is IIfcFaceBasedSurfaceModel) return engine.CreateSolidSet(repItem as IIfcFaceBasedSurfaceModel);
            else if (repItem is IIfcBoundingBox) { sSet.Add(engine.CreateSolid(repItem as IIfcBoundingBox, logger)); return sSet; }
            else if (repItem is IIfcSectionedSpine) { sSet.Add(engine.CreateSolid(repItem as IIfcSectionedSpine, logger)); return sSet; }
            else if (repItem is IIfcCsgPrimitive3D) { sSet.Add(engine.CreateSolid(repItem as IIfcCsgPrimitive3D, logger)); return sSet; }
            else return null;
        }
    }
}
