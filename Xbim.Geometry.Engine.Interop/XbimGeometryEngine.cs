using System;
using System.IO;
using System.Reflection;

using Xbim.Common;
using Xbim.Common.Geometry;
using Microsoft.Extensions.Logging;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;


namespace Xbim.Geometry.Engine.Interop
{  
    public class XbimGeometryEngine : IXbimGeometryEngine
    {

        private readonly IXbimGeometryEngine _engine;

        static XbimGeometryEngine()
        {
            // We need to wire in a custom assembly resolver since Xbim.Geometry.Engine is 
            // not located using standard probing rules (due to way we deploy processor specific binaries)
            AppDomain.CurrentDomain.AssemblyResolve += XbimCustomAssemblyResolver.ResolverHandler;
        }

        public XbimGeometryEngine()
        {

            // Warn if runtime for Engine is not present, this is not necessary any more as we are net47
           // XbimPrerequisitesValidator.Validate();

            var conventions = new XbimArchitectureConventions();    // understands the process we run under
            string assemblyName = $"{conventions.ModuleName}.dll";// + conventions.Suffix; dropping the use of a suffix
            try
            {               
                var ass =  Assembly.Load(assemblyName);
                var oh = Activator.CreateInstance(ass.FullName, "Xbim.Geometry.XbimGeometryCreator");           
                _engine = oh.Unwrap() as IXbimGeometryEngine; 
            }
            catch (Exception e)
            {
                throw new FileLoadException("Failed to load Xbim.Geometry.Engine.dll",e);
            }
             
        }
        public IXbimGeometryObject Create(IIfcGeometricRepresentationItem ifcRepresentation, ILogger logger)
        {
            return _engine.Create(ifcRepresentation, null,logger);
        }

        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection,
            double angle, XbimGeometryType storageType, ILogger logger)
        {
            return _engine.CreateShapeGeometry(geometryObject, precision, deflection, angle, storageType,logger);
        }

        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection, double angle, ILogger logger)
        {
            return _engine.CreateShapeGeometry(geometryObject,  precision,  deflection,  angle, XbimGeometryType.Polyhedron,logger);
        }
        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection, ILogger logger /*, angle = 0.5*/)
        {
            return _engine.CreateShapeGeometry(geometryObject, precision, deflection, 0.5, XbimGeometryType.Polyhedron,logger);
        }

        public IXbimSolid CreateSolid(IIfcSweptAreaSolid ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcExtrudedAreaSolid ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRevolvedAreaSolid ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSweptDiskSolid ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBoundingBox ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSurfaceCurveSweptAreaSolid ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBooleanClippingResult ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBooleanOperand ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcHalfSpaceSolid ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcPolygonalBoundedHalfSpace ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBoxedHalfSpace ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IIfcManifoldSolidBrep ifcSolid, ILogger logger)
        {
            return _engine.CreateSolidSet(ifcSolid,logger);
        }

        public IXbimSolidSet CreateSolidSet(IIfcFacetedBrep ifcSolid, ILogger logger)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IIfcFacetedBrepWithVoids ifcSolid, ILogger logger)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IIfcClosedShell ifcSolid, ILogger logger)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcCsgPrimitive3D ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcCsgSolid ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSphere ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBlock ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRightCircularCylinder ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRightCircularCone ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRectangularPyramid ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSweptDiskSolidPolygonal ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRevolvedAreaSolidTapered ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcFixedReferenceSweptAreaSolid ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcAdvancedBrep ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcAdvancedBrepWithVoids ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSectionedSpine ifcSolid, ILogger logger)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimShell CreateShell(IIfcOpenShell shell, ILogger logger)
        {
            return _engine.CreateShell(shell);
        }

        public IXbimShell CreateShell(IIfcConnectedFaceSet shell, ILogger logger)
        {
            return _engine.CreateShell(shell);
        }

        public IXbimShell CreateShell(IIfcSurfaceOfLinearExtrusion linExt, ILogger logger)
        {
            return _engine.CreateShell(linExt);
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcTriangulatedFaceSet shell, ILogger logger)
        {
            return _engine.CreateSurfaceModel(shell);
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcShellBasedSurfaceModel ifcSurface, ILogger logger)
        {
            return _engine.CreateSurfaceModel(ifcSurface);
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcFaceBasedSurfaceModel ifcSurface, ILogger logger)
        {
            return _engine.CreateSurfaceModel(ifcSurface);
        }

        public IXbimFace CreateFace(IIfcProfileDef profileDef, ILogger logger)
        {
            return _engine.CreateFace(profileDef);
        }

       
        public IXbimFace CreateFace(IIfcCompositeCurve cCurve, ILogger logger)
        {
            return _engine.CreateFace(cCurve);

        }
        public IXbimFace CreateFace(IIfcPolyline pline, ILogger logger)
        {
            return _engine.CreateFace(pline);

        }

        public IXbimFace CreateFace(IIfcPolyLoop loop, ILogger logger)
        {
            return _engine.CreateFace(loop);
        }


        public IXbimFace CreateFace(IIfcSurface surface, ILogger logger)
        {
            return _engine.CreateFace(surface);

        }

        public IXbimFace CreateFace(IIfcPlane plane, ILogger logger)
        {
            return _engine.CreateFace(plane);

        }
        public IXbimFace CreateFace(IXbimWire wire, ILogger logger)
        {
            return _engine.CreateFace(wire);

        }

        public IXbimWire CreateWire(IIfcCurve curve, ILogger logger)
        {
            return _engine.CreateWire(curve);
        }

        public IXbimWire CreateWire(IIfcCompositeCurveSegment compCurveSeg, ILogger logger)
        {
            return _engine.CreateWire(compCurveSeg);
        }

       

        public IXbimPoint CreatePoint(double x, double y, double z, double tolerance)
        {
            return _engine.CreatePoint(x, y, z, tolerance);
        }

        public IXbimPoint CreatePoint(IIfcCartesianPoint p)
        {
            return _engine.CreatePoint(p);
        }

        public IXbimPoint CreatePoint(XbimPoint3D p, double tolerance)
        {
            return _engine.CreatePoint(p, tolerance);
        }

        public IXbimPoint CreatePoint(IIfcPoint pt)
        {
            return _engine.CreatePoint(pt);
        }

        public IXbimPoint CreatePoint(IIfcPointOnCurve p, ILogger logger)
        {
            return _engine.CreatePoint(p);
        }

        public IXbimPoint CreatePoint(IIfcPointOnSurface p, ILogger logger)
        {
            return _engine.CreatePoint(p);
        }

        public IXbimVertex CreateVertexPoint(XbimPoint3D point, double precision)
        {
            return _engine.CreateVertexPoint(point, precision);
        }


        public IXbimSolidSet CreateSolidSet()
        {
            return _engine.CreateSolidSet();
        }

        public IXbimSolidSet CreateSolidSet(IIfcBooleanResult boolOp, ILogger logger)
        {
            return _engine.CreateSolidSet(boolOp);
        }

        public IXbimSolidSet CreateGrid(IIfcGrid grid, ILogger logger)
        {
            return _engine.CreateGrid(grid);
        }

        public void WriteTriangulation(TextWriter tw, IXbimGeometryObject shape, double tolerance, double deflection)
        {
            WriteTriangulation(tw, shape, tolerance, deflection: deflection, angle: 0.5);
        }

        public void WriteTriangulation(TextWriter tw, IXbimGeometryObject shape, double tolerance, double deflection, double angle)
        {
            _engine.WriteTriangulation(tw, shape, tolerance, deflection, angle);
        }
        public void WriteTriangulation(BinaryWriter bw, IXbimGeometryObject shape, double tolerance, double deflection, double angle)
        {
            _engine.WriteTriangulation(bw, shape, tolerance, deflection, angle);
        }

        public void Mesh(IXbimMeshReceiver receiver, IXbimGeometryObject geometryObject, double precision, double deflection,
            double angle = 0.5)
        {
            _engine.Mesh(receiver, geometryObject, precision, deflection, angle);
        }

       
        public void WriteTriangulation(BinaryWriter bw, IXbimGeometryObject shape, double tolerance, double deflection)
        {
            WriteTriangulation(bw, shape, tolerance, deflection: deflection, angle: 0.5);
        }


        public IXbimGeometryObject Create(IIfcGeometricRepresentationItem ifcRepresentation, IIfcAxis2Placement3D objectLocation, ILogger logger)
        {
            try
            {
                return _engine.Create(ifcRepresentation, objectLocation,logger);
            }
            catch (Exception e)
            {
                logger.LogError("EE001: Failed to create geometry #{0} of type {1}, {2}", ifcRepresentation.EntityLabel, ifcRepresentation.GetType().Name, e.Message);
                return null;
            }

        }

        public IXbimGeometryObjectSet CreateGeometryObjectSet()
        {
            return _engine.CreateGeometryObjectSet();
        }

        public IXbimCurve CreateCurve(IIfcCurve curve, ILogger logger)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcPolyline ifcPolyline, ILogger logger)
        {
            return _engine.CreateCurve(ifcPolyline);
        }

        public IXbimCurve CreateCurve(IIfcCircle curve, ILogger logger)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcEllipse curve, ILogger logger)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcLine curve, ILogger logger)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcTrimmedCurve curve, ILogger logger)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcBSplineCurveWithKnots curve, ILogger logger)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcRationalBSplineCurveWithKnots curve, ILogger logger)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcOffsetCurve3D curve, ILogger logger)
        {
            return _engine.CreateCurve(curve);
        }
        public IXbimCurve CreateCurve(IIfcOffsetCurve2D curve, ILogger logger)
        {
            return _engine.CreateCurve(curve);
        }

        public XbimMatrix3D ToMatrix3D(IIfcObjectPlacement objPlacement)
        {
            return _engine.ToMatrix3D(objPlacement);
        }

        /// <summary>
        /// Transforms an object geomtrically and returns a new object
        /// </summary>
        /// <param name="geometry"></param>
        /// <param name="cartesianTransform"></param>
        /// <returns></returns>
        public IXbimGeometryObject Transformed(IXbimGeometryObject geometry, IIfcCartesianTransformationOperator cartesianTransform)
        {
           return _engine.Transformed(geometry, cartesianTransform);
        }


        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcAxis2Placement3D placement)
        { 
            return _engine.Moved(geometryObject, placement);
        }
        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcAxis2Placement2D placement)
        {
            return _engine.Moved(geometryObject, placement);
        }
        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcPlacement placement)
        {
            return _engine.Moved(geometryObject, placement);
        }

        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcObjectPlacement objectPlacement)
        {
            return _engine.Moved(geometryObject, objectPlacement);
        }

        public IXbimGeometryObject FromBrep(string brepStr)
        {
            return _engine.FromBrep(brepStr);
        }

       

        public string ToBrep(IXbimGeometryObject geometryObject)
        {
            return _engine.ToBrep(geometryObject);
        }

       
    }
}
