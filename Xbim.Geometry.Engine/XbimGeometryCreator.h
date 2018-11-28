#pragma once
#include "XbimVertex.h"
#include "XbimVertex.h"
#include "XbimEdge.h"
using namespace System;
using namespace System::IO;
using namespace Xbim::Common;
using namespace Xbim::Common::Geometry;

using namespace System::Configuration;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Ifc4;


__declspec(dllexport) double __cdecl Load(void);

using namespace System::Reflection;

namespace Xbim
{
	namespace Geometry
	{

		public ref class XbimGeometryCreator : IXbimGeometryEngine
		{
			
			static Assembly^  ResolveHandler(Object^ /*Sender*/, ResolveEventArgs^ /*args*/)
			{
				
				// Warning: this should check the args for the assembly name!
				return nullptr;
			}
			bool Is3D(IIfcCurve^ rep);
			
		public:

			

		private:
			
			IXbimGeometryObject ^ Trim(XbimSetObject ^geometryObject);
			static XbimGeometryCreator()
			{
				//AppDomain::CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler(ResolveHandler);
				/*Assembly::Load("Xbim.Ifc4");
				Assembly::Load("Xbim.Common");
				Assembly::Load("Xbim.Tessellator");*/
				
				String^ timeOut = ConfigurationManager::AppSettings["BooleanTimeOut"];
				if (!double::TryParse(timeOut, BooleanTimeOut))
					BooleanTimeOut = 60;
				String^ ignoreIfcSweptDiskSolidParamsString = ConfigurationManager::AppSettings["IgnoreIfcSweptDiskSolidParams"];
				if(!bool::TryParse(ignoreIfcSweptDiskSolidParamsString,IgnoreIfcSweptDiskSolidParams))
					IgnoreIfcSweptDiskSolidParams = false;
				
			}
		protected:
			~XbimGeometryCreator()
			{
			}
				
		public:

			
			//Central point for logging all errors
			static void LogInfo(ILogger^ logger, Object^ entity, String^ format, ... array<Object^>^ arg);
			static void LogWarning(ILogger^ logger, Object^ entity, String^ format, ... array<Object^>^ arg);
			static void LogError(ILogger^ logger, Object^ entity, String^ format, ... array<Object^>^ arg);
			static void LogDebug(ILogger^ logger, Object^ entity, String^ format, ... array<Object^>^ arg);

			static double BooleanTimeOut;
			static bool IgnoreIfcSweptDiskSolidParams;
			virtual XbimShapeGeometry^ CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle, XbimGeometryType storageType, ILogger^ logger);
			virtual XbimShapeGeometry^ CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, ILogger^ logger/*, double angle = 0.5, XbimGeometryType storageType = XbimGeometryType::Polyhedron*/)
			{
				return CreateShapeGeometry(geometryObject, precision, deflection, 0.5, XbimGeometryType::Polyhedron,logger);
			};

			virtual IXbimGeometryObject^ Create(IIfcGeometricRepresentationItem^ geomRep, IIfcAxis2Placement3D^ objectLocation, ILogger^ logger);

			virtual void Mesh(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ geometry, double precision, double deflection, double angle);
			virtual void Mesh(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ geometry, double precision, double deflection/*, double angle = 0.5*/)
			{
				Mesh(mesh, geometry, precision, deflection, 0.5);
			};



			virtual IXbimGeometryObject^ Create(IIfcGeometricRepresentationItem^ geomRep, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ CreateGeometricSet(IIfcGeometricSet^ geomSet, ILogger^ logger);
			//Point Creation
			virtual IXbimPoint^ CreatePoint(double x, double y, double z, double tolerance);
			virtual IXbimPoint^ CreatePoint(IIfcCartesianPoint^ p);
			virtual IXbimPoint^ CreatePoint(XbimPoint3D p, double tolerance);
			virtual IXbimPoint^ CreatePoint(IIfcPoint^ pt);
			virtual IXbimPoint^ CreatePoint(IIfcPointOnCurve^ p, ILogger^ logger);
			virtual IXbimPoint^ CreatePoint(IIfcPointOnSurface^ p, ILogger^ logger);

			//Vertex Creation
			virtual IXbimVertex^ CreateVertex() { return gcnew XbimVertex(); }
			virtual IXbimVertex^ CreateVertexPoint(XbimPoint3D point, double precision) { return gcnew XbimVertex(point, precision); }

			//Edge Creation
			virtual IXbimEdge^ CreateEdge(IXbimVertex^ edgeStart, IXbimVertex^ edgeEnd) { return gcnew XbimEdge(edgeStart, edgeEnd); }

			//Create Wire
			virtual IXbimWire^ CreateWire(IIfcCurve^ curve, ILogger^ logger);
			virtual IXbimWire^ CreateWire(IIfcCompositeCurveSegment^ compCurveSeg, ILogger^ logger);
			//Face creation 
			virtual IXbimFace^ CreateFace(IIfcProfileDef ^ profile, ILogger^ logger);
			virtual IXbimFace^ CreateFace(IIfcCompositeCurve ^ cCurve, ILogger^ logger);
			virtual IXbimFace^ CreateFace(IIfcPolyline ^ pline, ILogger^ logger);
			virtual IXbimFace^ CreateFace(IIfcPolyLoop ^ loop, ILogger^ logger);
			virtual IXbimFace^ CreateFace(IIfcSurface ^ surface, ILogger^ logger);
			virtual IXbimFace^ CreateFace(IIfcPlane ^ plane, ILogger^ logger);
			virtual IXbimFace^ CreateFace(IXbimWire ^ wire, ILogger^ logger);

			//Shells creation
			virtual IXbimShell^ CreateShell(IIfcOpenShell^ shell, ILogger^ logger);
			virtual IXbimShell^ CreateShell(IIfcConnectedFaceSet^ shell, ILogger^ logger);
			virtual IXbimShell^ CreateShell(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^ logger);

#ifdef USE_CARVE_CSG
			virtual IXbimSolid^ CreateSolid(IXbimSolid^ from);
#endif // USE_CARVE_CSG


			//Solid creation 
			//static IXbimSolid^ CreateSolid(IIfcGeometricRepresentationItem^ IIfcSolid);
			//static IXbimSolid^ CreateSolid(IIfcSolidModel^ IIfcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcSweptAreaSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcExtrudedAreaSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcRevolvedAreaSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcSweptDiskSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcBoundingBox^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcSurfaceCurveSweptAreaSolid^ ifcSolid, ILogger^ logger);

			virtual IXbimSolid^ CreateSolid(IIfcBooleanResult^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcBooleanClippingResult^ ifcSolid, ILogger^ logger);
			
			virtual IXbimSolid^ CreateSolid(IIfcHalfSpaceSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcPolygonalBoundedHalfSpace^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcBoxedHalfSpace^ ifcSolid, ILogger^ logger);

			virtual IXbimSolidSet^ CreateSolidSet(IIfcManifoldSolidBrep^ ifcSolid, ILogger^ logger);

			virtual IXbimSolidSet^ CreateSolidSet(IIfcFacetedBrep^ ifcSolid, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcFacetedBrepWithVoids^ ifcSolid, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcClosedShell^ ifcSolid, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcSweptAreaSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcExtrudedAreaSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcRevolvedAreaSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcSurfaceCurveSweptAreaSolid^ ifcSolid, ILogger^ logger);

			virtual IXbimSolidSet^ CreateSolidSet(IIfcTriangulatedFaceSet^ shell, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcTriangulatedFaceSet^ shell, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^ logger);

			virtual IXbimSolid^ CreateSolid(IIfcCsgPrimitive3D^ ifcSolid, ILogger^ logger);
			
			virtual IXbimSolid^ CreateSolid(IIfcSphere^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcBlock^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcRightCircularCylinder^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcRightCircularCone^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcRectangularPyramid^ ifcSolid, ILogger^ logger);


			//Surface Models containing one or more faces, shells or solids
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^ logger);
			//Read and write functions
			virtual void WriteTriangulation(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);
			virtual void WriteTriangulation(TextWriter^ tw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);
			virtual void WriteTriangulation(BinaryWriter^ bw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);

			virtual IIfcFacetedBrep^ CreateFacetedBrep(Xbim::Common::IModel^ model, IXbimSolid^ solid);
			//Creates collections of objects
			virtual IXbimSolidSet^ CreateSolidSet();
			virtual IXbimSolidSet^ CreateSolidSet(IIfcBooleanResult^ boolOp, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcCsgSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcBooleanOperand^ ifcSolid, ILogger^ logger);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcBooleanClippingResult^ ifcSolid, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ CreateGeometryObjectSet();

			//Ifc4 interfaces
			virtual IXbimSolid^ CreateSolid(IIfcSweptDiskSolidPolygonal^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcRevolvedAreaSolidTapered^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcFixedReferenceSweptAreaSolid^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcAdvancedBrep^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcAdvancedBrepWithVoids^ ifcSolid, ILogger^ logger);
			virtual IXbimSolid^ CreateSolid(IIfcSectionedSpine^ ifcSolid, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IIfcTriangulatedFaceSet^ shell, ILogger^ logger);

			//Curves
			virtual IXbimCurve^ CreateCurve(IIfcCurve^ curve, ILogger^ logger);
			virtual IXbimCurve^ CreateCurve(IIfcPolyline^ curve, ILogger^ logger);
			virtual IXbimCurve^ CreateCurve(IIfcCircle^ curve, ILogger^ logger);
			virtual IXbimCurve^ CreateCurve(IIfcEllipse^ curve, ILogger^ logger);
			virtual IXbimCurve^ CreateCurve(IIfcLine^ curve, ILogger^ logger);
			virtual IXbimCurve^ CreateCurve(IIfcTrimmedCurve^ curve, ILogger^ logger);
			virtual IXbimCurve^ CreateCurve(IIfcRationalBSplineCurveWithKnots^ curve, ILogger^ logger);
			virtual IXbimCurve^ CreateCurve(IIfcBSplineCurveWithKnots^ curve, ILogger^ logger);
			virtual IXbimCurve^ CreateCurve(IIfcOffsetCurve3D^ curve, ILogger^ logger);
			virtual IXbimCurve^ CreateCurve(IIfcOffsetCurve2D^ curve, ILogger^ logger);
			virtual XbimMatrix3D ToMatrix3D(IIfcObjectPlacement ^ objPlacement, ILogger^ logger);
			virtual IXbimSolidSet^ CreateGrid(IIfcGrid^ grid, ILogger^ logger);

			// Inherited via IXbimGeometryEngine
			virtual IXbimGeometryObject ^ Transformed(IXbimGeometryObject ^geometryObject, IIfcCartesianTransformationOperator ^transformation);
			virtual IXbimGeometryObject ^ Moved(IXbimGeometryObject ^geometryObject, IIfcPlacement ^placement);
			virtual IXbimGeometryObject ^ Moved(IXbimGeometryObject ^geometryObject, IIfcAxis2Placement3D ^placement) 
			{
				return Moved(geometryObject, (IIfcPlacement ^)placement);
			};
			virtual IXbimGeometryObject ^ Moved(IXbimGeometryObject ^geometryObject, IIfcAxis2Placement2D ^placement)
			{
				return Moved(geometryObject, (IIfcPlacement ^)placement);
			};
			virtual IXbimGeometryObject ^ Moved(IXbimGeometryObject ^geometryObject, IIfcObjectPlacement ^objectPlacement, ILogger^ logger);
			virtual IXbimGeometryObject^ FromBrep(String^ brepStr);
			virtual String^ ToBrep(IXbimGeometryObject^ geometryObject);
		};
	}
}