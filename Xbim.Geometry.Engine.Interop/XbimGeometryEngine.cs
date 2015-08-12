using System;
using System.IO;
using System.Runtime.Remoting;

using Xbim.Common.Geometry;
using Xbim.Common.Logging;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.ProfileResource;
using Xbim.Ifc2x3.TopologyResource;
using Xbim.IO; 
using Xbim.XbimExtensions.SelectTypes;
using XbimGeometry.Interfaces;

namespace Xbim.Geometry.Engine.Interop
{  
    public class XbimGeometryEngine : IXbimGeometryCreator
    {

        private readonly IXbimGeometryCreator _engine;

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
            _engine = oh.Unwrap() as IXbimGeometryCreator;   
        }
        public IXbimGeometryObject Create(IfcGeometricRepresentationItem ifcRepresentation)
        {
            return Create(ifcRepresentation, null);
        }

        public IXbimShapeGeometryData CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection,
            double angle, XbimGeometryType storageType)
        {
            return _engine.CreateShapeGeometry(geometryObject, precision, deflection, angle, storageType);
        }

        public IXbimShapeGeometryData CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection, double angle)
        {
            return _engine.CreateShapeGeometry(geometryObject,  precision,  deflection,  angle, XbimGeometryType.Polyhedron);
        }
        public IXbimShapeGeometryData CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection /*, angle = 0.5*/)
        {
            return _engine.CreateShapeGeometry(geometryObject, precision, deflection, 0.5, XbimGeometryType.Polyhedron);
        }

        public IXbimSolid CreateSolid(IfcSweptAreaSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcExtrudedAreaSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcRevolvedAreaSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcSweptDiskSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcBoundingBox ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcSurfaceCurveSweptAreaSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcBooleanClippingResult ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcBooleanOperand ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcHalfSpaceSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcPolygonalBoundedHalfSpace ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcBoxedHalfSpace ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IfcManifoldSolidBrep ifcSolid)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IfcFacetedBrep ifcSolid)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IfcFacetedBrepWithVoids ifcSolid)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolidSet CreateSolidSet(IfcClosedShell ifcSolid)
        {
            return _engine.CreateSolidSet(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcCsgPrimitive3D ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcCsgSolid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcSphere ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcBlock ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcRightCircularCylinder ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcRightCircularCone ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimSolid CreateSolid(IfcRectangularPyramid ifcSolid)
        {
            return _engine.CreateSolid(ifcSolid);
        }

        public IXbimShell CreateShell(IfcOpenShell shell)
        {
            return _engine.CreateShell(shell);
        }

        public IXbimShell CreateShell(IfcConnectedFaceSet shell)
        {
            return _engine.CreateShell(shell);
        }

        public IXbimShell CreateShell(IfcSurfaceOfLinearExtrusion linExt)
        {
            return _engine.CreateShell(linExt);
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IfcShellBasedSurfaceModel ifcSurface)
        {
            return _engine.CreateSurfaceModel(ifcSurface);
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IfcFaceBasedSurfaceModel ifcSurface)
        {
            return _engine.CreateSurfaceModel(ifcSurface);
        }

        public IXbimFace CreateFace(IfcProfileDef profileDef)
        {
            return _engine.CreateFace(profileDef);
        }

       
        public IXbimFace CreateFace(IfcCompositeCurve cCurve)
        {
            return _engine.CreateFace(cCurve);

        }
        public IXbimFace CreateFace(IfcPolyline pline)
        {
            return _engine.CreateFace(pline);

        }

        public IXbimFace CreateFace(IfcPolyLoop loop)
        {
            return _engine.CreateFace(loop);
        }


        public IXbimFace CreateFace(IfcSurface surface)
        {
            return _engine.CreateFace(surface);

        }

        public IXbimFace CreateFace(IfcPlane plane)
        {
            return _engine.CreateFace(plane);

        }
        public IXbimFace CreateFace(IXbimWire wire)
        {
            return _engine.CreateFace(wire);

        }

        public IXbimWire CreateWire(IfcCurve curve)
        {
            return _engine.CreateWire(curve);
        }

        public IXbimWire CreateWire(IfcCompositeCurveSegment compCurveSeg)
        {
            return _engine.CreateWire(compCurveSeg);
        }

        public IXbimPoint CreatePoint(double x, double y, double z, double tolerance)
        {
            return _engine.CreatePoint(x, y, z, tolerance);
        }

        public IXbimPoint CreatePoint(IfcCartesianPoint p)
        {
            return _engine.CreatePoint(p);
        }

        public IXbimPoint CreatePoint(XbimPoint3D p, double tolerance)
        {
            return _engine.CreatePoint(p, tolerance);
        }

        public IXbimPoint CreatePoint(IfcPoint pt)
        {
            return _engine.CreatePoint(pt);
        }

        public IXbimPoint CreatePoint(IfcPointOnCurve p)
        {
            return _engine.CreatePoint(p);
        }

        public IXbimPoint CreatePoint(IfcPointOnSurface p)
        {
            return _engine.CreatePoint(p);
        }

        public IXbimVertex CreateVertexPoint(XbimPoint3D point, double precision)
        {
            return _engine.CreateVertexPoint(point, precision);
        }

        public IfcFacetedBrep CreateFacetedBrep(XbimModel model, IXbimSolid solid)
        {
            return _engine.CreateFacetedBrep(model, solid);
        }

        public IXbimSolidSet CreateSolidSet()
        {
            return _engine.CreateSolidSet();
        }

        public IXbimSolidSet CreateSolidSet(IfcBooleanResult boolOp)
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



        public IXbimGeometryObject Create(IfcGeometricRepresentationItem ifcRepresentation, IfcAxis2Placement3D objectLocation)
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
