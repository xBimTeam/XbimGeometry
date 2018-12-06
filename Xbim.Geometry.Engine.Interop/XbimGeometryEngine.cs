using Microsoft.Extensions.Logging;
using System;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Interop
{
    public class XbimGeometryEngine : IXbimGeometryEngine
    {
        private readonly IXbimGeometryEngine _engine;

        private readonly ILogger<XbimGeometryEngine> _logger;

        static XbimGeometryEngine()
        {

            // We need to wire in a custom assembly resolver since Xbim.Geometry.Engine is 
            // not located using standard probing rules (due to way we deploy processor specific binaries)
            AppDomain.CurrentDomain.AssemblyResolve += XbimCustomAssemblyResolver.ResolverHandler;
        }

        public XbimGeometryEngine() : this(null)
        { }

        public XbimGeometryEngine(ILogger<XbimGeometryEngine> logger)
        {

            // Warn if runtime for Engine is not present, this is not necessary any more as we are net47
            //XbimPrerequisitesValidator.Validate();


            _logger = logger ?? XbimLogging.CreateLogger<XbimGeometryEngine>();

            var conventions = new XbimArchitectureConventions();    // understands the process we run under
            string assemblyName = $"{conventions.ModuleName}.dll";// + conventions.Suffix; dropping the use of a suffix
            _logger.LogDebug("Loading {assemblyName}", assemblyName);
            try
            {
                var ass = Assembly.Load(assemblyName);
                _logger.LogTrace("Loaded {fullName} from {codebase}", ass.GetName().FullName, ass.CodeBase);
                var t = ass.GetType("Xbim.Geometry.XbimGeometryCreator");
                var obj = Activator.CreateInstance(t);
                _logger.LogTrace("Created Instance of {fullName}", obj.GetType().FullName);
                if (obj == null)
                {
                    throw new Exception("Failed to create Geometry Engine");
                }

                _engine = obj as IXbimGeometryEngine;
                if (_engine == null)
                {
                    throw new Exception("Failed to cast Geometry Engine to IXbimGeometryEngine");
                }

                _logger.LogDebug("XbimGeometryEngine constructed successfully");
            }
            catch (Exception e)
            {
                _logger.LogError(0, e, "Failed to construct XbimGeometryEngine");
                throw new FileLoadException($"Failed to load Xbim.Geometry.Engine{conventions.Suffix}.dll", e);
            }

        }

        public IXbimGeometryObject Create(IIfcGeometricRepresentationItem ifcRepresentation, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcRepresentation))
            {
                return _engine.Create(ifcRepresentation, null, logger);
            }
        }

        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection,
            double angle, XbimGeometryType storageType, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return _engine.CreateShapeGeometry(geometryObject, precision, deflection, angle, storageType, logger);
            }
        }

        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection, double angle, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return _engine.CreateShapeGeometry(geometryObject, precision, deflection, angle, XbimGeometryType.Polyhedron, logger);
            }
        }
        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection, ILogger logger /*, angle = 0.5*/)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return _engine.CreateShapeGeometry(geometryObject, precision, deflection, 0.5, XbimGeometryType.Polyhedron, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSweptAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcExtrudedAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRevolvedAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSweptDiskSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcBoundingBox ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSurfaceCurveSweptAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcBooleanClippingResult ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcBooleanOperand ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcHalfSpaceSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcPolygonalBoundedHalfSpace ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcBoxedHalfSpace ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcManifoldSolidBrep ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcFacetedBrep ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcFacetedBrepWithVoids ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcClosedShell ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcCsgPrimitive3D ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcCsgSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSphere ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcBlock ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRightCircularCylinder ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRightCircularCone ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRectangularPyramid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSweptDiskSolidPolygonal ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRevolvedAreaSolidTapered ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcFixedReferenceSweptAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcAdvancedBrep ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcAdvancedBrepWithVoids ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSectionedSpine ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimShell CreateShell(IIfcOpenShell shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return _engine.CreateShell(shell, logger);
            }
        }

        public IXbimShell CreateShell(IIfcConnectedFaceSet shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return _engine.CreateShell(shell, logger);
            }
        }

        public IXbimShell CreateShell(IIfcSurfaceOfLinearExtrusion linExt, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, linExt))
            {
                return _engine.CreateShell(linExt, logger);
            }
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcTriangulatedFaceSet shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return _engine.CreateSurfaceModel(shell, logger);
            }
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcShellBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return _engine.CreateSurfaceModel(ifcSurface, logger);
            }
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcFaceBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return _engine.CreateSurfaceModel(ifcSurface, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcTriangulatedFaceSet shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return _engine.CreateSolidSet(shell, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcShellBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return _engine.CreateSolidSet(ifcSurface, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcFaceBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return _engine.CreateSolidSet(ifcSurface, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcTriangulatedFaceSet shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return _engine.CreateSolid(shell, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcShellBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return _engine.CreateSolid(ifcSurface, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcFaceBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return _engine.CreateSolid(ifcSurface, logger);
            }
        }

        public IXbimFace CreateFace(IIfcProfileDef profileDef, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, profileDef))
            {
                return _engine.CreateFace(profileDef, logger);
            }
        }


        public IXbimFace CreateFace(IIfcCompositeCurve cCurve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, cCurve))
            {
                return _engine.CreateFace(cCurve, logger);
            }
        }
        public IXbimFace CreateFace(IIfcPolyline pline, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, pline))
            {
                return _engine.CreateFace(pline, logger);
            }
        }

        public IXbimFace CreateFace(IIfcPolyLoop loop, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, loop))
            {
                return _engine.CreateFace(loop, logger);
            }
        }


        public IXbimFace CreateFace(IIfcSurface surface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, surface))
            {
                return _engine.CreateFace(surface, logger);
            }
        }

        public IXbimFace CreateFace(IIfcPlane plane, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, plane))
            {
                return _engine.CreateFace(plane, logger);
            }
        }

        public IXbimFace CreateFace(IXbimWire wire, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, wire))
            {
                return _engine.CreateFace(wire, logger);
            }
        }

        public IXbimWire CreateWire(IIfcCurve curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateWire(curve, logger);
            }
        }

        public IXbimWire CreateWire(IIfcCompositeCurveSegment compCurveSeg, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, compCurveSeg))
            {
                return _engine.CreateWire(compCurveSeg, logger);
            }
        }



        public IXbimPoint CreatePoint(double x, double y, double z, double tolerance)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger))
            {
                return _engine.CreatePoint(x, y, z, tolerance);
            }
        }

        public IXbimPoint CreatePoint(IIfcCartesianPoint p)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, p))
            {
                return _engine.CreatePoint(p);
            }
        }

        public IXbimPoint CreatePoint(XbimPoint3D p, double tolerance)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, p))
            {
                return _engine.CreatePoint(p, tolerance);
            }
        }

        public IXbimPoint CreatePoint(IIfcPoint pt)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, pt))
            {
                return _engine.CreatePoint(pt);
            }
        }

        public IXbimPoint CreatePoint(IIfcPointOnCurve p, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, p))
            {
                return _engine.CreatePoint(p, logger);
            }
        }

        public IXbimPoint CreatePoint(IIfcPointOnSurface p, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, p))
            {
                return _engine.CreatePoint(p, logger);
            }
        }

        public IXbimVertex CreateVertexPoint(XbimPoint3D point, double precision)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, point))
            {
                return _engine.CreateVertexPoint(point, precision);
            }
        }


        public IXbimSolidSet CreateSolidSet()
        {
            try
            {
                using (new Tracer(LogHelper.CurrentFunctionName(), this._logger))
                {
                    return _engine.CreateSolidSet();
                }
            }
            catch (Exception e)
            {
                _logger.LogError(0, e, "Failed in CreateSolidSet");
                throw new Exception("Engine is not valid", e);
            }

        }

        public IXbimSolidSet CreateSolidSet(IIfcBooleanResult boolOp, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, boolOp))
            {
                return _engine.CreateSolidSet(boolOp, logger);
            }
        }

        public IXbimSolidSet CreateGrid(IIfcGrid grid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, grid))
            {
                return _engine.CreateGrid(grid, logger);
            }
        }

        public void WriteTriangulation(TextWriter tw, IXbimGeometryObject shape, double tolerance, double deflection)
        {
            WriteTriangulation(tw, shape, tolerance, deflection: deflection, angle: 0.5);
        }

        public void WriteTriangulation(TextWriter tw, IXbimGeometryObject shape, double tolerance, double deflection, double angle)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shape))
            {
                _engine.WriteTriangulation(tw, shape, tolerance, deflection, angle);
            }
        }
        public void WriteTriangulation(BinaryWriter bw, IXbimGeometryObject shape, double tolerance, double deflection, double angle)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shape))
            {
                _engine.WriteTriangulation(bw, shape, tolerance, deflection, angle);
            }
        }

        public void Mesh(IXbimMeshReceiver receiver, IXbimGeometryObject geometryObject, double precision, double deflection,
            double angle = 0.5)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                _engine.Mesh(receiver, geometryObject, precision, deflection, angle);
            }
        }


        public void WriteTriangulation(BinaryWriter bw, IXbimGeometryObject shape, double tolerance, double deflection)
        {
            WriteTriangulation(bw, shape, tolerance, deflection: deflection, angle: 0.5);
        }


        public IXbimGeometryObject Create(IIfcGeometricRepresentationItem ifcRepresentation, IIfcAxis2Placement3D objectLocation, ILogger logger)
        {
            try
            {
                using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcRepresentation))
                {
                    return _engine.Create(ifcRepresentation, objectLocation, logger);
                }
            }
            catch (Exception e)
            {
                (logger ?? _logger).LogError("EE001: Failed to create geometry #{0} of type {1}, {2}", ifcRepresentation.EntityLabel, ifcRepresentation.GetType().Name, e.Message);
                return null;
            }

        }

        public IXbimGeometryObjectSet CreateGeometryObjectSet()
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger))
            {
                return _engine.CreateGeometryObjectSet();
            }
        }

        public IXbimCurve CreateCurve(IIfcCurve curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcPolyline ifcPolyline, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcPolyline))
            {
                return _engine.CreateCurve(ifcPolyline, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcCircle curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcEllipse curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcLine curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcTrimmedCurve curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcBSplineCurveWithKnots curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcRationalBSplineCurveWithKnots curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcOffsetCurve3D curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateCurve(curve, logger);
            }
        }
        public IXbimCurve CreateCurve(IIfcOffsetCurve2D curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return _engine.CreateCurve(curve, logger);
            }
        }

        public XbimMatrix3D ToMatrix3D(IIfcObjectPlacement objPlacement, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, objPlacement))
            {
                return _engine.ToMatrix3D(objPlacement, logger);
            }
        }

        /// <summary>
        /// Transforms an object geomtrically and returns a new object
        /// </summary>
        /// <param name="geometry"></param>
        /// <param name="cartesianTransform"></param>
        /// <returns></returns>
        public IXbimGeometryObject Transformed(IXbimGeometryObject geometry, IIfcCartesianTransformationOperator cartesianTransform)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometry))
            {
                return _engine.Transformed(geometry, cartesianTransform);
            }
        }


        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcAxis2Placement3D placement)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return _engine.Moved(geometryObject, placement);
            }
        }
        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcAxis2Placement2D placement)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return _engine.Moved(geometryObject, placement);
            }
        }
        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcPlacement placement)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return _engine.Moved(geometryObject, placement);
            }
        }

        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcObjectPlacement objectPlacement, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return _engine.Moved(geometryObject, objectPlacement, logger);
            }
        }

        public IXbimGeometryObject FromBrep(string brepStr)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger))
            {
                return _engine.FromBrep(brepStr);
            }
        }

        public string ToBrep(IXbimGeometryObject geometryObject)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return _engine.ToBrep(geometryObject);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcSweptAreaSolid ifcSolid, ILogger logger = null)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return _engine.CreateSolidSet(ifcSolid, logger);
            }
        }
    }

    public static class LogHelper
    {
        public static string CurrentFunctionName([CallerMemberName] string caller = "")
        {
            return caller;
        }

    }

    /// <summary>
    /// Traces method calls
    /// </summary>
    internal class Tracer : IDisposable
    {
        private readonly string methodName;
        private readonly ILogger logger;

        public Tracer(string methodName, ILogger logger)
        {
            this.methodName = methodName;
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
            logger.LogTrace("Entering GeometryEngine {function}", methodName);
        }

        public Tracer(string methodName, ILogger logger, IPersistEntity entity)
        {
            this.methodName = methodName;
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
            if (logger.IsEnabled(LogLevel.Trace))   // Optimisation to avoid GetType reflection unless Trace enabled
            {
                logger.LogTrace("Entering GeometryEngine {function} with #{entity} [{type}]",
                methodName, entity.EntityLabel, entity.GetType().Name);
            }
        }

        public Tracer(string methodName, ILogger logger, IXbimGeometryObject geometryObject)
        {
            this.methodName = methodName;
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
            if (logger.IsEnabled(LogLevel.Trace)) // Optimisation to avoid GetType reflection unless Trace enabled
            {
                logger.LogTrace("Entering GeometryEngine {function} with {tag} [{type}]",
                    methodName, geometryObject.Tag, geometryObject.GetType().Name);
            }
        }

        public Tracer(string methodName, ILogger logger, XbimPoint3D point)
        {
            this.methodName = methodName;
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
            logger.LogTrace("Entering GeometryEngine {function} with point {x},{y},{z}", methodName, point.X, point.Y, point.Z);
        }

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    logger.LogTrace("Exiting GeometryEngine {function}", methodName);
                }

                disposedValue = true;
            }
        }

        ~Tracer()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(false);
        }

        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
            GC.SuppressFinalize(this);  // To avoid excessive GC
        }
        #endregion



    }
}
