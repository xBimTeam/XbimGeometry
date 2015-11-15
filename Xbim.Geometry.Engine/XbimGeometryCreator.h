#pragma once
#include "XbimVertex.h"
#include "XbimVertex.h"
#include "XbimEdge.h"

using namespace System;
using namespace System::IO;
using namespace Xbim::IO;
using namespace Xbim::Common::Logging;
using namespace Xbim::Common::Geometry;

using namespace System::Configuration;
using namespace Xbim::Ifc4::Interfaces;

namespace Xbim
{
	namespace Geometry
	{

		public ref class XbimGeometryCreator : IXbimGeometryEngine
		{
		protected:
			~XbimGeometryCreator()
			{
			}

		public:
			
			static  property bool SupportsFacetedShapes{bool get()
				{
#ifdef USE_CARVE_CSG
					return true;
#else
					return false;
#endif // USE_CARVE_CSG
				}
			}
			static XbimGeometryCreator()
			{
				String^ timeOut = ConfigurationManager::AppSettings["BooleanTimeOut"];
				if (!double::TryParse(timeOut, BooleanTimeOut))
					BooleanTimeOut = 240;
			}
			//Central point for logging all errors
			static ILogger^ logger = LoggerFactory::GetLogger();
			static double BooleanTimeOut;
			virtual property ILogger^ Logger{ILogger^ get(){ return XbimGeometryCreator::logger; }};
			virtual IXbimShapeGeometryData^ CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle, XbimGeometryType storageType);
			virtual IXbimShapeGeometryData^ CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection/*, double angle = 0.5, XbimGeometryType storageType = XbimGeometryType::Polyhedron*/)
			{
				return CreateShapeGeometry(geometryObject, precision, deflection, 0.5, XbimGeometryType::Polyhedron);
			};

			virtual IXbimGeometryObject^ Create(IIfcGeometricRepresentationItem^ geomRep, IIfcAxis2Placement3D^ objectLocation);

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
			virtual IXbimVertex^ CreateVertex(){ return gcnew XbimVertex(); }
			virtual IXbimVertex^ CreateVertexPoint(XbimPoint3D point, double precision){ return gcnew XbimVertex(point, precision); }

			//Edge Creation
			virtual IXbimEdge^ CreateEdge(IXbimVertex^ edgeStart, IXbimVertex^ edgeEnd){ return gcnew XbimEdge(edgeStart, edgeEnd); }

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
			virtual IXbimShell^ CreateShell(IIfcTriangulatedFaceSet^ shell);
		};
	}
}