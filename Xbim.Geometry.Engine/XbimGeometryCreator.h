#pragma once
#include "XbimVertex.h"
#include "XbimVertex.h"
#include "XbimEdge.h"

using namespace System;
using namespace System::IO;
using namespace Xbim::IO;
using namespace Xbim::Ifc2x3::IO;
using namespace Xbim::IO::Esent;
using namespace Xbim::Common::Logging;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc2x3::GeometricModelResource;
using namespace Xbim::Ifc2x3::TopologyResource;
using namespace Xbim::Ifc2x3::ProfileResource;
using namespace Xbim::Ifc2x3::GeometryResource;

using namespace System::Configuration;
namespace Xbim
{
	namespace Geometry
	{

		public ref class XbimGeometryCreator : Xbim::Ifc2x3::IO::IXbimGeometryCreator
		{
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

			virtual IXbimGeometryObject^ Create(IfcGeometricRepresentationItem^ geomRep, IfcAxis2Placement3D^ objectLocation);

			virtual IXbimGeometryObject^ Create(IfcGeometricRepresentationItem^ geomRep);
			virtual IXbimGeometryObjectSet^ CreateGeometricSet(IfcGeometricSet^ geomSet);
			//Point Creation
			virtual IXbimPoint^ CreatePoint(double x, double y, double z, double tolerance);
			virtual IXbimPoint^ CreatePoint(IfcCartesianPoint^ p);
			virtual IXbimPoint^ CreatePoint(XbimPoint3D p, double tolerance);
			virtual IXbimPoint^ CreatePoint(IfcPoint^ pt);
			virtual IXbimPoint^ CreatePoint(IfcPointOnCurve^ p);
			virtual IXbimPoint^ CreatePoint(IfcPointOnSurface^ p);
			
			//Vertex Creation
			virtual IXbimVertex^ CreateVertex(){ return gcnew XbimVertex(); }
			virtual IXbimVertex^ CreateVertexPoint(XbimPoint3D point, double precision){ return gcnew XbimVertex(point, precision); }

			//Edge Creation
			virtual IXbimEdge^ CreateEdge(IXbimVertex^ edgeStart, IXbimVertex^ edgeEnd){ return gcnew XbimEdge(edgeStart, edgeEnd); }

			//Create Wire
			virtual IXbimWire^ CreateWire(IfcCurve^ curve);
			virtual IXbimWire^ CreateWire(IfcCompositeCurveSegment^ compCurveSeg);
			//Face creation 
			virtual IXbimFace^ CreateFace(IfcProfileDef ^ profile);
			virtual IXbimFace^ CreateFace(IfcCompositeCurve ^ cCurve);
			virtual IXbimFace^ CreateFace(IfcPolyline ^ pline);
			virtual IXbimFace^ CreateFace(IfcPolyLoop ^ loop);
			virtual IXbimFace^ CreateFace(IfcSurface ^ surface);
			virtual IXbimFace^ CreateFace(IfcPlane ^ plane);
			virtual IXbimFace^ CreateFace(IXbimWire ^ wire);

			//Shells creation
			virtual IXbimShell^ CreateShell(IfcOpenShell^ shell);
			virtual IXbimShell^ CreateShell(IfcConnectedFaceSet^ shell);
			virtual IXbimShell^ CreateShell(IfcSurfaceOfLinearExtrusion^ linExt);
			
#ifdef USE_CARVE_CSG
			virtual IXbimSolid^ CreateSolid(IXbimSolid^ from);	
#endif // USE_CARVE_CSG


			//Solid creation 
			//static IXbimSolid^ CreateSolid(IfcGeometricRepresentationItem^ ifcSolid);
			//static IXbimSolid^ CreateSolid(IfcSolidModel^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcSweptAreaSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcExtrudedAreaSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcRevolvedAreaSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcSweptDiskSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcBoundingBox^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcSurfaceCurveSweptAreaSolid^ ifcSolid);

			virtual IXbimSolid^ CreateSolid(IfcBooleanClippingResult^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcBooleanOperand^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcHalfSpaceSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcPolygonalBoundedHalfSpace^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcBoxedHalfSpace^ ifcSolid);
			
			virtual IXbimSolidSet^ CreateSolidSet(IfcManifoldSolidBrep^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IfcFacetedBrep^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IfcFacetedBrepWithVoids^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IfcClosedShell^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IfcSweptAreaSolid^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IfcExtrudedAreaSolid^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IfcRevolvedAreaSolid^ ifcSolid);
			virtual IXbimSolidSet^ CreateSolidSet(IfcSurfaceCurveSweptAreaSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcCsgPrimitive3D^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcCsgSolid^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcSphere^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcBlock^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcRightCircularCylinder^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcRightCircularCone^ ifcSolid);
			virtual IXbimSolid^ CreateSolid(IfcRectangularPyramid^ ifcSolid);


			//Surface Models containing one or more faces, shells or solids
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IfcShellBasedSurfaceModel^ ifcSurface);
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IfcFaceBasedSurfaceModel^ ifcSurface);
			//Read and write functions
			virtual void WriteTriangulation(TextWriter^ tw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);
			virtual void WriteTriangulation(BinaryWriter^ bw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);
			//Reads a triangulate data store, if untriangulate is true coplanar faces are removed

#ifdef USE_CARVE_CSG
			virtual IXbimGeometryObject^ ReadTriangulation(TextReader^ tr, bool unTriangulate);
			virtual IXbimGeometryObject^ ReadTriangulation(TextReader^ tr/*, bool unTriangulate = false*/);
			virtual IXbimSolid^ CreateFacetedSolid(IfcBooleanClippingResult^ ifcSolid);
#endif // USE_CARVE_CSG


#ifdef USE_CARVE_CSG
			//Faceted mesh Creation
			virtual IXbimSolid^ CreateFacetedSolid(IXbimSolid^ solid, double precision, double deflection);
			virtual IXbimSolid^ CreateFacetedSolid(IXbimSolid^ solid, double precision, double deflection, double angle);


			//Creates a faceted solid of triangles
			virtual IXbimSolid^ CreateTriangulatedSolid(IXbimSolid^ solid, double precision, double deflection);
			virtual IXbimSolid^ CreateTriangulatedSolid(IXbimSolid^ solid, double precision, double deflection, double angle);

#endif // USE_CARVE_CSG

			virtual IfcFacetedBrep^ CreateFacetedBrep(Xbim::Ifc2x3::IO::XbimModel^ model, IXbimSolid^ solid);
			//Creates collections of objects
			virtual IXbimSolidSet^ CreateSolidSet();
			virtual IXbimSolidSet^ CreateSolidSet(IfcBooleanResult^ boolOp);

			virtual IXbimSolidSet^ CreateBooleanResult(IfcBooleanResult^ clip);
			
			virtual IXbimGeometryObjectSet^ CreateGeometryObjectSet();
			
};
	}
}