using System;
using System.IO;
using System.Runtime.Remoting;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
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
            string assemblyName = "Xbim.Geometry.Engine" + conventions.Suffix;

            ObjectHandle oh = Activator.CreateInstance(assemblyName, "Xbim.Geometry.XbimGeometryCreator");
            _engine = oh.Unwrap() as IXbimGeometryEngine;   
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

        public IXbimShell CreateShell(IIfcTriangulatedFaceSet shell)
        {
            return _engine.CreateShell(shell);
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

        

    }
}
