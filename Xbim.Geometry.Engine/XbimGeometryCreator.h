#pragma once
#include "XbimVertex.h"
#include "XbimVertex.h"
#include "XbimEdge.h"
using namespace System;
using namespace System::IO;
using namespace Xbim::Common;
using namespace Xbim::Common::Logging;
using namespace Xbim::Common::Geometry;

using namespace System::Configuration;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Ifc4;
namespace Xbim
{
	namespace Geometry
	{

		public ref class XbimGeometryCreator : IXbimGeometryEngine
		{
		private:
			IXbimGeometryObject ^ Trim(XbimSetObject ^geometryObject);
		protected:
			~XbimGeometryCreator()
			{
			}
			bool Is3D(IIfcCurve^ rep);
			static ILogger^ logger = LoggerFactory::GetLogger(); ;
		public:

			static XbimGeometryCreator()
			{
				String^ timeOut = ConfigurationManager::AppSettings["BooleanTimeOut"];
				if (!double::TryParse(timeOut, BooleanTimeOut))
					BooleanTimeOut = 180;
			}
			//Central point for logging all errors
			static void LogInfo(Object^ entity, String^ format, ... array<Object^>^ arg);
			static void LogWarning(Object^ entity, String^ format, ... array<Object^>^ arg);
			static void LogError(Object^ entity, String^ format, ... array<Object^>^ arg);
			static void LogDebug(Object^ entity, String^ format, ... array<Object^>^ arg);

			static double BooleanTimeOut;
			virtual property ILogger^ Logger {ILogger^ get() { return XbimGeometryCreator::logger; }};
			virtual XbimShapeGeometry^ CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle, XbimGeometryType storageType);
			virtual XbimShapeGeometry^ CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection/*, double angle = 0.5, XbimGeometryType storageType = XbimGeometryType::Polyhedron*/)
			{
				return CreateShapeGeometry(geometryObject, precision, deflection, 0.5, XbimGeometryType::Polyhedron);
			};

			virtual IXbimGeometryObject^ Create(IIfcGeometricRepresentationItem^ geomRep, IIfcAxis2Placement3D^ objectLocation);

			virtual void Mesh(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ geometry, double precision, double deflection, double angle);
			virtual void Mesh(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ geometry, double precision, double deflection/*, double angle = 0.5*/)
			{
				Mesh(mesh, geometry, precision, deflection, 0.5);
			};



			virtual IXbimGeometryObject^ Create(IIfcGeometricRepresentationItem^ geomRep);
			virtual IXbimGeometryObjectSet^ CreateGeometricSet(IIfcGeometricSet^ geomSet);
			//Point Creation
			virtual IXbimPoint^ CreatePoint(double x, double y, double z, double tolerance);
			virtual IXbimPoint^ CreatePoint(IIfcCartesianPoint^ p);
			virtual IXbimPoint^ CreatePoint(XbimPoint3D p, double tolerance);
			virtual IXbimPoint^ CreatePoint(IIfcPoint^ pt);
			virtual IXbimPoint^ CreatePoint(IIfcPointOnCurve^ p);
			virtual IXbimPoint^ CreatePoint(IIfcPointOnSurface^ p);

			//Vertex Creation
			virtual IXbimVertex^ CreateVertex() { return gcnew XbimVertex(); }
			virtual IXbimVertex^ CreateVertexPoint(XbimPoint3D point, double precision) { return gcnew XbimVertex(point, precision); }

			//Edge Creation
			virtual IXbimEdge^ CreateEdge(IXbimVertex^ edgeStart, IXbimVertex^ edgeEnd) { return gcnew XbimEdge(edgeStart, edgeEnd); }

			//Create Wire
			virtual IXbimWire^ CreateWire(IIfcCurve^ curve);
			virtual IXbimWire^ CreateWire(IIfcCompositeCurveSegment^ compCurveSeg);
			//Face creation 
			virtual IXbimFace^ CreateFace(IIfcProfileDef ^ profile);
			virtual IXbimFace^ CreateFace(IIfcCompositeCurve ^ cCurve);
			virtual IXbimFace^ CreateFace(IIfcPolyline ^ pline);
			virtual IXbimFace^ CreateFace(IIfcPolyLoop ^ loop);
			virtual IXbimFace^ CreateFace(IIfcSurface ^ surface);
			virtual IXbimFace^ CreateFace(IIfcPlane ^ plane);
			virtual IXbimFace^ CreateFace(IXbimWire ^ wire);

			//Shells creation
			virtual IXbimShell^ CreateShell(IIfcOpenShell^ shell);
			virtual IXbimShell^ CreateShell(IIfcConnectedFaceSet^ shell);
			virtual IXbimShell^ CreateShell(IIfcSurfaceOfLinearExtrusion^ linExt);

#ifdef USE_CARVE_CSG
			virtual IXbimSolid^ CreateSolid(IXbimSolid^ from);
#endif // USE_CARVE_CSG


			//Solid creation 
			//static IXbimSolid^ CreateSolid(IIfcGeometricRepresentationItem^ IIfcSolid);
			//static IXbimSolid^ CreateSolid(IIfcSolidModel^ IIfcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcSweptAreaSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcExtrudedAreaSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcRevolvedAreaSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcSweptDiskSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcBoundingBox^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcSurfaceCurveSweptAreaSolid^ ifcSolid);

			virtual IXbimSolid^ CreateSolid(IIfcBooleanClippingResult^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcBooleanOperand^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcHalfSpaceSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcPolygonalBoundedHalfSpace^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcBoxedHalfSpace^ ifcSolid);

			virtual IXbimSolidSet^ CreateSolidSet(IIfcManifoldSolidBrep^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcFacetedBrep^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcFacetedBrepWithVoids^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcClosedShell^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcSweptAreaSolid^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcExtrudedAreaSolid^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcRevolvedAreaSolid^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcSurfaceCurveSweptAreaSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcCsgPrimitive3D^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcCsgSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcSphere^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcBlock^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcRightCircularCylinder^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcRightCircularCone^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcRectangularPyramid^ ifcSolid);


			//Surface Models containing one or more faces, shells or solids
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IIfcShellBasedSurfaceModel^ ifcSurface);
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IIfcFaceBasedSurfaceModel^ ifcSurface);
			//Read and write functions
			virtual void WriteTriangulation(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);
			virtual void WriteTriangulation(TextWriter^ tw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);
			virtual void WriteTriangulation(BinaryWriter^ bw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);

			virtual IIfcFacetedBrep^ CreateFacetedBrep(Xbim::Common::IModel^ model, IXbimSolid^ solid);
			//Creates collections of objects
			virtual IXbimSolidSet^ CreateSolidSet();
			virtual IXbimSolidSet^ CreateSolidSet(IIfcBooleanResult^ boolOp);
			virtual IXbimSolidSet^ CreateBooleanResult(IIfcBooleanResult^ clip);
			virtual IXbimGeometryObjectSet^ CreateGeometryObjectSet();

			//Ifc4 interfaces
			virtual IXbimSolid^ CreateSolid(IIfcSweptDiskSolidPolygonal^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcRevolvedAreaSolidTapered^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcFixedReferenceSweptAreaSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcAdvancedBrep^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcAdvancedBrepWithVoids^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IIfcSectionedSpine^ ifcSolid);
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IIfcTriangulatedFaceSet^ shell);

			//Curves
			virtual IXbimCurve^ CreateCurve(IIfcCurve^ curve);
			virtual IXbimCurve^ CreateCurve(IIfcPolyline^ curve);
			virtual IXbimCurve^ CreateCurve(IIfcCircle^ curve);
			virtual IXbimCurve^ CreateCurve(IIfcEllipse^ curve);
			virtual IXbimCurve^ CreateCurve(IIfcLine^ curve);
			virtual IXbimCurve^ CreateCurve(IIfcTrimmedCurve^ curve);
			virtual IXbimCurve^ CreateCurve(IIfcRationalBSplineCurveWithKnots^ curve);
			virtual IXbimCurve^ CreateCurve(IIfcBSplineCurveWithKnots^ curve);
			virtual IXbimCurve^ CreateCurve(IIfcOffsetCurve3D^ curve);
			virtual IXbimCurve^ CreateCurve(IIfcOffsetCurve2D^ curve);
			virtual XbimMatrix3D ToMatrix3D(IIfcObjectPlacement ^ objPlacement);
			virtual IXbimSolidSet^ CreateGrid(IIfcGrid^ grid);

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
			virtual IXbimGeometryObject ^ Moved(IXbimGeometryObject ^geometryObject, IIfcObjectPlacement ^objectPlacement);
			virtual IXbimGeometryObject^ FromBrep(String^ brepStr);
			virtual String^ ToBrep(IXbimGeometryObject^ geometryObject);
		};
	}
}