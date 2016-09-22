using System;
using System.IO;
using System.Reflection;
using System.Runtime.Remoting;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
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

            // Warn if runtime for Engine is not present
            XbimPrerequisitesValidator.Validate();

            var conventions = new XbimArchitectureConventions();    // understands the process we run under
            string assemblyName = conventions.ModuleName + conventions.Suffix;
            try
            {               
                var ass =  Assembly.Load(assemblyName);
                var oh = Activator.CreateInstance(ass.FullName, "Xbim.Geometry.XbimGeometryCreator");           
                _engine = oh.Unwrap() as IXbimGeometryEngine; 
            }
            catch (Exception e)
            {

                throw e;
            }
             
        }
        public IXbimGeometryObject Create(IIfcGeometricRepresentationItem ifcRepresentation)
        {
            return Create(ifcRepresentation, null);
        }

        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection,
            double angle, XbimGeometryType storageType)
        {
            return _engine.CreateShapeGeometry(geometryObject, precision, deflection, angle, storageType);
        }

        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection, double angle)
        {
            return _engine.CreateShapeGeometry(geometryObject,  precision,  deflection,  angle, XbimGeometryType.Polyhedron);
        }
        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection /*, angle = 0.5*/)
        {
            return _engine.CreateShapeGeometry(geometryObject, precision, deflection, 0.5, XbimGeometryType.Polyhedron);
        }

        public IXbimSolid CreateSolid(IIfcSweptAreaSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcExtrudedAreaSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRevolvedAreaSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSweptDiskSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBoundingBox ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSurfaceCurveSweptAreaSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBooleanClippingResult ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBooleanOperand ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcHalfSpaceSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcPolygonalBoundedHalfSpace ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBoxedHalfSpace ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IIfcManifoldSolidBrep ifcSolid)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IIfcFacetedBrep ifcSolid)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IIfcFacetedBrepWithVoids ifcSolid)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IIfcClosedShell ifcSolid)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcCsgPrimitive3D ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcCsgSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSphere ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcBlock ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRightCircularCylinder ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRightCircularCone ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRectangularPyramid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSweptDiskSolidPolygonal ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcRevolvedAreaSolidTapered ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcFixedReferenceSweptAreaSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcAdvancedBrep ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcAdvancedBrepWithVoids ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IIfcSectionedSpine ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimShell CreateShell(IIfcOpenShell shell)
        {
            return _engine.CreateShell(shell);
        }

        public IXbimShell CreateShell(IIfcConnectedFaceSet shell)
        {
            return _engine.CreateShell(shell);
        }

        public IXbimShell CreateShell(IIfcSurfaceOfLinearExtrusion linExt)
        {
            return _engine.CreateShell(linExt);
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcTriangulatedFaceSet shell)
        {
            return _engine.CreateSurfaceModel(shell);
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcShellBasedSurfaceModel ifcSurface)
        {
            return _engine.CreateSurfaceModel(ifcSurface);
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcFaceBasedSurfaceModel ifcSurface)
        {
            return _engine.CreateSurfaceModel(ifcSurface);
        }

        public IXbimFace CreateFace(IIfcProfileDef profileDef)
        {
            return _engine.CreateFace(profileDef);
        }

       
        public IXbimFace CreateFace(IIfcCompositeCurve cCurve)
        {
            return _engine.CreateFace(cCurve);

        }
        public IXbimFace CreateFace(IIfcPolyline pline)
        {
            return _engine.CreateFace(pline);

        }

        public IXbimFace CreateFace(IIfcPolyLoop loop)
        {
            return _engine.CreateFace(loop);
        }


        public IXbimFace CreateFace(IIfcSurface surface)
        {
            return _engine.CreateFace(surface);

        }

        public IXbimFace CreateFace(IIfcPlane plane)
        {
            return _engine.CreateFace(plane);

        }
        public IXbimFace CreateFace(IXbimWire wire)
        {
            return _engine.CreateFace(wire);

        }

        public IXbimWire CreateWire(IIfcCurve curve)
        {
            return _engine.CreateWire(curve);
        }

        public IXbimWire CreateWire(IIfcCompositeCurveSegment compCurveSeg)
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

        public IXbimPoint CreatePoint(IIfcPointOnCurve p)
        {
            return _engine.CreatePoint(p);
        }

        public IXbimPoint CreatePoint(IIfcPointOnSurface p)
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

        public IXbimSolidSet CreateSolidSet(IIfcBooleanResult boolOp)
        {
            return _engine.CreateSolidSet(boolOp);
        }

        public IXbimSolidSet CreateGrid(IIfcGrid grid)
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

       

        public ILogger Logger
        {
            get { return _engine.Logger; }
        }



        public IXbimGeometryObject Create(IIfcGeometricRepresentationItem ifcRepresentation, IIfcAxis2Placement3D objectLocation)
        {
            try
            {
                return _engine.Create(ifcRepresentation, objectLocation);
            }
            catch (AccessViolationException e)
            {

                Logger.ErrorFormat("EE001: Failed to create geometry #{0} of type {1}, {2]", ifcRepresentation.EntityLabel, ifcRepresentation.GetType().Name, e.Message);
                return null;
            }
        }

        public IXbimGeometryObjectSet CreateGeometryObjectSet()
        {
            return _engine.CreateGeometryObjectSet();
        }

        public IXbimCurve CreateCurve(IIfcCurve curve)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcPolyline ifcPolyline)
        {
            return _engine.CreateCurve(ifcPolyline);
        }

        public IXbimCurve CreateCurve(IIfcCircle curve)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcEllipse curve)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcLine curve)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcTrimmedCurve curve)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcBSplineCurveWithKnots curve)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcRationalBSplineCurveWithKnots curve)
        {
            return _engine.CreateCurve(curve);
        }

        public IXbimCurve CreateCurve(IIfcOffsetCurve3D curve)
        {
            return _engine.CreateCurve(curve);
        }
        public IXbimCurve CreateCurve(IIfcOffsetCurve2D curve)
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
