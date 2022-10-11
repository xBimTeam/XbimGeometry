#pragma once
#include <TopoDS_Solid.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include "XbimOccShape.h"
#include "XbimWire.h"
#include "XbimFace.h"
#include "XbimFaceSet.h"

using namespace System::Collections::Generic;
using namespace System::IO;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::MeasureResource;
namespace Xbim
{
	namespace Geometry
	{
		static void BuildIfcSurfaceCurveSweptAreaSolid(TopoDS_Wire& sweepOcc, TopoDS_Face& refSurface, TopoDS_Face& faceStartOcc, double precision, TopoDS_Solid & result,  int& retflag);
		ref class XbimSolidV5 :IXbimSolid, XbimOccShape
		{
		private:

			System::IntPtr ptrContainer;
			virtual property TopoDS_Solid* pSolid
			{
				TopoDS_Solid* get() sealed { return (TopoDS_Solid*)ptrContainer.ToPointer(); }
				void set(TopoDS_Solid* val)sealed { ptrContainer = System::IntPtr(val); }
			}
			void InstanceCleanup();

			double SegLength(IIfcCompositeCurveSegment^ segment, ILogger^ logger);



#pragma region Initialisers

			void Init(IIfcSolidModel^ solid, ILogger^ logger);
			void Init(IIfcManifoldSolidBrep^ solid, ILogger^ logger);
			void Init(IIfcSweptAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger);
			void Init(IIfcExtrudedAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger);
			void Init(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger);
			
			void Init(IIfcRevolvedAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger);

			void Init(IIfcExtrudedAreaSolidTapered^ solid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger);
			void Init(IIfcRevolvedAreaSolidTapered^ solid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger);
			void Init(IIfcSectionedSpine^ solid, ILogger^ logger);


			void Init(IIfcSweptDiskSolid^ solid, ILogger^ logger);
			System::String^ BuildSweptDiskSolid(const TopoDS_Wire& directrixWire, double radius, double innerRadius, BRepBuilderAPI_TransitionMode transitionMode);
			XbimWireV5^ CreateDirectrix(IIfcCurve^ directrix, System::Nullable<IfcParameterValue> startParam, System::Nullable<IfcParameterValue> endParam, ILogger^ logger);
			// this is case handled by IIfcSweptDiskSolid 
			// void Init(IIfcSweptDiskSolidPolygonal^ solid, ILogger^ logger);
			void Init(IIfcBoundingBox^ solid, ILogger^ logger);
			void Init(IIfcHalfSpaceSolid^ solid, ILogger^ logger);
			void Init(IIfcBoxedHalfSpace^ solid, ILogger^ logger);
			void Init(IIfcPolygonalBoundedHalfSpace^ solid, ILogger^ logger);



			void Init(XbimRect3D rect3D, double tolerance, ILogger^ logger);

			void Init(IIfcFixedReferenceSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger);
			void Init(IIfcCsgPrimitive3D^ IIfcSolid, ILogger^ logger);

			void Init(IIfcSphere^ IIfcSolid, ILogger^ logger);
			void Init(IIfcBlock^ IIfcSolid, ILogger^ logger);
			void Init(IIfcRightCircularCylinder^ IIfcSolid, ILogger^ logger);
			void Init(IIfcRightCircularCone^ IIfcSolid, ILogger^ logger);
			void Init(IIfcRectangularPyramid^ IIfcSolid, ILogger^ logger);
			void Init(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger);
			void Init(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger);
			void Init(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger);
#pragma endregion

		public:

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimSolidV5^ left, XbimSolidV5^ right);
			static bool operator !=(XbimSolidV5^ left, XbimSolidV5^ right);
			virtual bool Equals(IXbimSolid^ s);
#pragma endregion

#pragma region IXbimSolid Interface
			virtual property bool IsEmpty {bool get(); }
			virtual property bool IsValid {bool get() override { return pSolid != nullptr && !pSolid->IsNull(); }; }
			virtual property  XbimGeometryObjectType GeometryType {XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimSolidType; }; }
			virtual property IXbimShellSet^ Shells { IXbimShellSet^ get(); }
			virtual property IXbimFaceSet^ Faces { IXbimFaceSet^ get(); }
			virtual property IXbimEdgeSet^ Edges { IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices {IXbimVertexSet^ get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
			virtual property bool IsClosed {bool get(); }
			virtual property double Volume {double get(); }
			virtual property double SurfaceArea { double get(); }
			virtual property bool IsPolyhedron { bool get(); }
			virtual property bool HasValidTopology {bool get(); }
			virtual IXbimSolidSet^ Cut(IXbimSolid^ toCut, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Union(IXbimSolid^ toUnion, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Intersection(IXbimSolid^ toIntersect, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Cut(IXbimSolidSet^ toCut, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Union(IXbimSolidSet^ toUnion, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Intersection(IXbimSolidSet^ toIntersect, double tolerance, ILogger^ logger);
			virtual IXbimFaceSet^ Section(IXbimFace^ face, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual void SaveAsBrep(System::String^ fileName);
#pragma endregion

#pragma region destructors

			~XbimSolidV5() { InstanceCleanup(); }
			!XbimSolidV5() { InstanceCleanup(); }

#pragma endregion

#pragma region constructors
			XbimSolidV5() {};
			XbimSolidV5(const TopoDS_Solid& solid);
			XbimSolidV5(const TopoDS_Solid& solid, Object^ tag);
			XbimSolidV5(IIfcSolidModel^ solid, ILogger^ logger);
			XbimSolidV5(IIfcManifoldSolidBrep^ solid, ILogger^ logger);
			XbimSolidV5(IIfcSweptAreaSolid^ solid, ILogger^ logger);
			XbimSolidV5(IIfcSweptAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger);
			XbimSolidV5(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolidV5(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^ logger);

			XbimSolidV5(IIfcHalfSpaceSolid^ solid, ILogger^ logger);
			XbimSolidV5(IIfcBoxedHalfSpace^ solid, ILogger^ logger);
			XbimSolidV5(IIfcPolygonalBoundedHalfSpace^ solid, ILogger^ logger);
			XbimSolidV5(IIfcExtrudedAreaSolid^ solid, ILogger^ logger);
			XbimSolidV5(IIfcExtrudedAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolidV5(IIfcExtrudedAreaSolidTapered^ solid, ILogger^ logger);
			XbimSolidV5(IIfcExtrudedAreaSolidTapered^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolidV5(IIfcRevolvedAreaSolidTapered^ solid, ILogger^ logger);
			XbimSolidV5(IIfcRevolvedAreaSolidTapered^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolidV5(IIfcRevolvedAreaSolid^ solid, ILogger^ logger);
			XbimSolidV5(IIfcRevolvedAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolidV5(IIfcSweptDiskSolid^ solid, ILogger^ logger);
			// XbimSolid(IIfcSweptDiskSolidPolygonal^ solid, ILogger^ logger);
			XbimSolidV5(IIfcSectionedSpine^ solid, ILogger^ logger);
			XbimSolidV5(IIfcBoundingBox^ solid, ILogger^ logger);


			XbimSolidV5(IIfcFixedReferenceSweptAreaSolid^ solid, ILogger^ logger);
			XbimSolidV5(IIfcFixedReferenceSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolidV5(IIfcCsgPrimitive3D^ IIfcSolid, ILogger^ logger);

			XbimSolidV5(IIfcSphere^ IIfcSolid, ILogger^ logger);
			XbimSolidV5(IIfcBlock^ IIfcSolid, ILogger^ logger);
			XbimSolidV5(IIfcRightCircularCylinder^ IIfcSolid, ILogger^ logger);
			XbimSolidV5(IIfcRightCircularCone^ IIfcSolid, ILogger^ logger);
			XbimSolidV5(IIfcRectangularPyramid^ IIfcSolid, ILogger^ logger);
			XbimSolidV5(XbimRect3D rect3D, double tolerance, ILogger^ logger);
			XbimSolidV5(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger);
			XbimSolidV5(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger);
			XbimSolidV5(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger);
#pragma endregion


#pragma region operators
			operator const TopoDS_Solid& () { return *pSolid; }
			virtual operator const TopoDS_Shape& () override { return *pSolid; }
#pragma endregion



#pragma region Methods
			//moves the solid to the new position
			void Move(IIfcAxis2Placement3D^ position);
			void Translate(XbimVector3D translation);
			void Reverse();
			void CorrectOrientation();
			bool FixTopology(double tolerance);
#pragma endregion



			// Inherited via XbimOccShape
			virtual XbimGeometryObject^ Transformed(IIfcCartesianTransformationOperator^ transformation) override;


			// Inherited via XbimOccShape
			virtual XbimGeometryObject^ Moved(IIfcPlacement^ placement) override;

			virtual XbimGeometryObject^ Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger) override;
			virtual void Move(TopLoc_Location loc);
		};

	}
}