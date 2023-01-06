#pragma once
#include "./Services//ModelGeometryService.h"
#include "Services/ShapeService.h"
using namespace Xbim::Geometry::Services;
namespace Xbim
{
	namespace Geometry
	{

		public ref class XbimGeometryCreatorV6 : public ModelGeometryService, IXGeometryEngineV6
		{


			bool Is3D(IIfcCurve^ rep);

		public:

			static System::String^ SurfaceOfLinearExtrusion = "#SurfaceOfLinearExtrusion";
			static System::String^ PolylineTrimLengthOneForEntireLine = "#PolylineTrimLengthOneForEntireLine";
			ShapeService^ _shapeService;
		private:

			IXbimGeometryObject^ Trim(XbimSetObject^ geometryObject);

			static XbimGeometryCreatorV6()
			{
				//AppDomain::CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler(ResolveHandler);
				/*Assembly::Load("Xbim.Ifc4");
				Assembly::Load("Xbim.Common");
				Assembly::Load("Xbim.Tessellator");*/

				System::String^ timeOut = System::Environment::GetEnvironmentVariable("BooleanTimeOut");
				if (!int::TryParse(timeOut, BooleanTimeOut))
					BooleanTimeOut = 60;
				System::String^ fuzzyString = System::Environment::GetEnvironmentVariable("FuzzyFactor");
				if (!double::TryParse(fuzzyString, FuzzyFactor))
					FuzzyFactor = 10;

				System::String^ linearDeflection = System::Environment::GetEnvironmentVariable("LinearDeflectionInMM");
				if (!double::TryParse(linearDeflection, LinearDeflectionInMM))
					LinearDeflectionInMM = 50; //max chord diff

				System::String^ angularDeflection = System::Environment::GetEnvironmentVariable("AngularDeflectionInRadians");
				if (!double::TryParse(angularDeflection, AngularDeflectionInRadians))
					AngularDeflectionInRadians = 0.5;// deflection of 28 degrees

				System::String^ ignoreIfcSweptDiskSolidParamsString = System::Environment::GetEnvironmentVariable("IgnoreIfcSweptDiskSolidParams");
				if (!bool::TryParse(ignoreIfcSweptDiskSolidParamsString, IgnoreIfcSweptDiskSolidParams))
					IgnoreIfcSweptDiskSolidParams = false;

			}
		public:

			XbimGeometryCreatorV6(IModel^ model, ILoggerFactory^ loggerFactory) : Xbim::Geometry::Services::ModelGeometryService(model, loggerFactory)
			{
				_shapeService = gcnew ShapeService();
			}
			//Central point for logging all errors
			static void LogInfo(ILogger^ logger, Object^ entity, System::String^ format, ... array<Object^>^ arg);
			static void LogWarning(ILogger^ logger, Object^ entity, System::String^ format, ... array<Object^>^ arg);
			static void LogError(ILogger^ logger, Object^ entity, System::String^ format, ... array<Object^>^ arg);
			static void LogDebug(ILogger^ logger, Object^ entity, System::String^ format, ... array<Object^>^ arg);

			static int BooleanTimeOut;
			static double FuzzyFactor;
			static double LinearDeflectionInMM;
			static double AngularDeflectionInRadians;
			static bool IgnoreIfcSweptDiskSolidParams;
			IXModelGeometryService^ GetModelGeometryService() { return this; }
			virtual property  IXModelGeometryService^ ModelGeometryService{ IXModelGeometryService^  get() { return this; } }
			virtual IXbimGeometryObject^ Create(IIfcGeometricRepresentationItem^ geomRep, ILogger^);
			virtual IXShape^ Build(IIfcGeometricRepresentationItem^ geomRep);
			virtual IXbimGeometryObject^ Create(IIfcGeometricRepresentationItem^ geomRep, IIfcAxis2Placement3D^ objectLocation, ILogger^);
			virtual IXbimGeometryObjectSet^ CreateGeometricSet(IIfcGeometricSet^ geomSet, ILogger^);
			//Point Creation
			virtual IXbimPoint^ CreatePoint(double x, double y, double z, double tolerance);
			virtual IXbimPoint^ CreatePoint(IIfcCartesianPoint^ p);
			virtual IXbimPoint^ CreatePoint(XbimPoint3D p, double tolerance);
			virtual IXbimPoint^ CreatePoint(IIfcPoint^ pt);
			virtual IXbimPoint^ CreatePoint(IIfcPointOnCurve^ p, ILogger^);
			virtual IXbimPoint^ CreatePoint(IIfcPointOnSurface^ p, ILogger^);

			//Vertex Creation
			virtual IXbimVertex^ CreateVertex() { return gcnew XbimVertex(); }
			virtual IXbimVertex^ CreateVertexPoint(XbimPoint3D point, double precision) { return gcnew XbimVertex(point, precision); }

			//Edge Creation
			virtual IXbimEdge^ CreateEdge(IXbimVertex^ edgeStart, IXbimVertex^ edgeEnd) { return gcnew XbimEdge(edgeStart, edgeEnd); }

			//Create Wire
			virtual IXbimWire^ CreateWire(IIfcCurve^ curve, ILogger^);

			virtual IXbimWire^ CreateWire(IIfcCompositeCurveSegment^ compCurveSeg, ILogger^);
			//Face creation 
			virtual IXbimFace^ CreateFace(IIfcProfileDef^ profile, ILogger^);
			virtual IXbimFace^ CreateFace(IIfcCompositeCurve^ cCurve, ILogger^);
			virtual IXbimFace^ CreateFace(IIfcPolyline^ pline, ILogger^);
			virtual IXbimFace^ CreateFace(IIfcPolyLoop^ loop, ILogger^);
			virtual IXbimFace^ CreateFace(IIfcSurface^ surface, ILogger^);
			virtual IXbimFace^ CreateFace(IIfcPlane^ plane, ILogger^);
			virtual IXbimFace^ CreateFace(IXbimWire^ wire, ILogger^);

			//Shells creation
			virtual IXbimShell^ CreateShell(IIfcOpenShell^ shell, ILogger^);
			virtual IXbimShell^ CreateShell(IIfcConnectedFaceSet^ shell, ILogger^);
			virtual IXbimShell^ CreateShell(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^);


			virtual IXbimSolid^ CreateSolid(IIfcSweptAreaSolid^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcExtrudedAreaSolid^ ifcSolid, ILogger^);
			IXbimSolid^ CreateSolid(IIfcPolygonalFaceSet^ shell, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcRevolvedAreaSolid^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcSweptDiskSolid^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcBoundingBox^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcSurfaceCurveSweptAreaSolid^ ifcSolid, ILogger^);

			virtual IXbimSolid^ CreateSolid(IIfcBooleanResult^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcBooleanClippingResult^ ifcSolid, ILogger^);

			virtual IXbimSolid^ CreateSolid(IIfcHalfSpaceSolid^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcPolygonalBoundedHalfSpace^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcBoxedHalfSpace^ ifcSolid, ILogger^);

			virtual IXbimSolidSet^ CreateSolidSet(IIfcManifoldSolidBrep^ ifcSolid, ILogger^);

			virtual IXbimSolidSet^ CreateSolidSet(IIfcFacetedBrep^ ifcSolid, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcFacetedBrepWithVoids^ ifcSolid, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcClosedShell^ ifcSolid, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcSweptAreaSolid^ ifcSolid, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcExtrudedAreaSolid^ ifcSolid, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcRevolvedAreaSolid^ ifcSolid, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcSurfaceCurveSweptAreaSolid^ ifcSolid, ILogger^);

			virtual IXbimSolidSet^ CreateSolidSet(IIfcTriangulatedFaceSet^ shell, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcPolygonalFaceSet^ shell, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcTriangulatedFaceSet^ shell, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^);

			virtual IXbimSolid^ CreateSolid(IIfcCsgPrimitive3D^ ifcSolid, ILogger^);

			virtual IXbimSolid^ CreateSolid(IIfcSphere^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcBlock^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcRightCircularCylinder^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcRightCircularCone^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcRectangularPyramid^ ifcSolid, ILogger^);


			//Surface Models containing one or more faces, shells or solids
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^);
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^);
			

			virtual IIfcFacetedBrep^ CreateFacetedBrep(Xbim::Common::IModel^ model, IXbimSolid^ solid);
			//Creates collections of objects
			virtual IXbimSolidSet^ CreateSolidSet();
			virtual IXbimSolidSet^ CreateSolidSet(IIfcBooleanResult^ boolOp, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcCsgSolid^ ifcSolid, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcBooleanOperand^ ifcSolid, ILogger^);
			virtual IXbimSolidSet^ CreateSolidSet(IIfcBooleanClippingResult^ ifcSolid, ILogger^);
			virtual IXbimGeometryObjectSet^ CreateGeometryObjectSet();

			//Ifc4 interfaces
			virtual IXbimSolid^ CreateSolid(IIfcSweptDiskSolidPolygonal^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcRevolvedAreaSolidTapered^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcFixedReferenceSweptAreaSolid^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcAdvancedBrep^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcAdvancedBrepWithVoids^ ifcSolid, ILogger^);
			virtual IXbimSolid^ CreateSolid(IIfcSectionedSpine^ ifcSolid, ILogger^);
			virtual IXbimGeometryObjectSet^ CreateSurfaceModel(IIfcTessellatedFaceSet^ shell, ILogger^);

			//Curves
			virtual IXbimCurve^ CreateCurve(IIfcCurve^ curve, ILogger^);
			virtual IXbimCurve^ CreateCurve(IIfcPolyline^ curve, ILogger^);
			virtual IXbimCurve^ CreateCurve(IIfcCircle^ curve, ILogger^);
			virtual IXbimCurve^ CreateCurve(IIfcEllipse^ curve, ILogger^);
			virtual IXbimCurve^ CreateCurve(IIfcLine^ curve, ILogger^);
			virtual IXbimCurve^ CreateCurve(IIfcTrimmedCurve^ curve, ILogger^);
			virtual IXbimCurve^ CreateCurve(IIfcRationalBSplineCurveWithKnots^ curve, ILogger^);
			virtual IXbimCurve^ CreateCurve(IIfcBSplineCurveWithKnots^ curve, ILogger^);
			virtual IXbimCurve^ CreateCurve(IIfcOffsetCurve3D^ curve, ILogger^);
			virtual IXbimCurve^ CreateCurve(IIfcOffsetCurve2D^ curve, ILogger^);
			
			//triangulation
			virtual XbimShapeGeometry^ CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle, XbimGeometryType storageType, ILogger^);

			virtual XbimShapeGeometry^ CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, ILogger^ logger/*, double angle = 0.5, XbimGeometryType storageType = XbimGeometryType::Polyhedron*/)
			{
				return CreateShapeGeometry(geometryObject, precision, deflection, 0.5, XbimGeometryType::PolyhedronBinary, logger);
			};

			virtual XbimShapeGeometry^ CreateShapeGeometry(double oneMillimetre, IXbimGeometryObject^ geometryObject, double precision, ILogger^ logger)
			{
				double linearDeflection = oneMillimetre * LinearDeflectionInMM;
				return CreateShapeGeometry(geometryObject, precision, linearDeflection, AngularDeflectionInRadians, XbimGeometryType::PolyhedronBinary, logger);
			};

			

			//XbimMesh^ CreateMeshGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle);

			virtual void Mesh(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ geometry, double precision, double deflection, double angle);
			virtual void Mesh(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ geometry, double precision, double deflection/*, double angle = 0.5*/)
			{
				Mesh(mesh, geometry, precision, deflection, 0.5);
			};

			
			//Misc
			virtual XbimMatrix3D ToMatrix3D(IIfcObjectPlacement^ objPlacement, ILogger^);
			virtual IXbimSolidSet^ CreateGrid(IIfcGrid^ grid, ILogger^);

			virtual IXbimGeometryObject^ Transformed(IXbimGeometryObject^ geometryObject, IIfcCartesianTransformationOperator^ transformation);
			virtual IXbimGeometryObject^ Moved(IXbimGeometryObject^ geometryObject, IIfcPlacement^ placement);
			virtual IXbimGeometryObject^ Moved(IXbimGeometryObject^ geometryObject, IIfcAxis2Placement3D^ placement)
			{
				return Moved(geometryObject, (IIfcPlacement^)placement);
			};
			virtual IXbimGeometryObject^ Moved(IXbimGeometryObject^ geometryObject, IIfcAxis2Placement2D^ placement)
			{
				return Moved(geometryObject, (IIfcPlacement^)placement);
			};
			virtual IXbimGeometryObject^ Moved(IXbimGeometryObject^ geometryObject, IIfcObjectPlacement^ objectPlacement, ILogger^);
			virtual IXbimGeometryObject^ FromBrep(System::String^ brepStr);
			virtual System::String^ ToBrep(IXbimGeometryObject^ geometryObject);
			virtual void WriteBrep(System::String^ fileName, IXbimGeometryObject^ brep);
			virtual  IXbimGeometryObject^ ReadBrep(System::String^ fileName);
			//Read and write functions
			virtual void WriteTriangulation(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);
			virtual void WriteTriangulation(TextWriter^ tw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);
			virtual void WriteTriangulation(BinaryWriter^ bw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle);
			array<System::Byte>^ WriteTriangulation(IXShape^ shape, double tolerance, double deflection, double angle, Bnd_Box& bounds);
		};
	}
}
