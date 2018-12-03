#pragma once
#include "XbimOccShape.h"
#include "XbimWire.h"
#include "XbimFace.h"
#include "XbimFaceSet.h"
#include <TopoDS_Solid.hxx>

using namespace System::Collections::Generic;
using namespace System::IO;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common::Geometry;

namespace Xbim
{
	namespace Geometry
	{

		ref class XbimSolid :IXbimSolid, XbimOccShape
		{
		private:
			
			IntPtr ptrContainer;
			virtual property TopoDS_Solid* pSolid
			{
				TopoDS_Solid* get() sealed { return (TopoDS_Solid*)ptrContainer.ToPointer(); }
				void set(TopoDS_Solid* val)sealed { ptrContainer = IntPtr(val); }
			}
			void InstanceCleanup();
			
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
			void Init(IIfcSweptDiskSolidPolygonal^ solid, ILogger^ logger);
			void Init(IIfcBoundingBox^ solid, ILogger^ logger);
			void Init(IIfcHalfSpaceSolid^ solid, double maxExtrusion, XbimPoint3D centroid, ILogger^ logger);
			void Init(IIfcBoxedHalfSpace^ solid, ILogger^ logger);
			void Init(IIfcPolygonalBoundedHalfSpace^ solid, double maxExtrusion, ILogger^ logger);
			
			
			
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
			static bool operator ==(XbimSolid^ left, XbimSolid^ right);
			static bool operator !=(XbimSolid^ left, XbimSolid^ right);
			virtual bool Equals(IXbimSolid^ s);
#pragma endregion

#pragma region IXbimSolid Interface
			virtual property bool IsEmpty {bool get(); }
			virtual property bool IsValid{bool get() override { return pSolid != nullptr && !pSolid->IsNull(); }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimSolidType; }; }
			virtual property IXbimShellSet^ Shells{ IXbimShellSet^ get(); }
			virtual property IXbimFaceSet^ Faces{ IXbimFaceSet^ get(); }
			virtual property IXbimEdgeSet^ Edges{ IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices{IXbimVertexSet^ get(); }
			virtual property XbimRect3D BoundingBox{XbimRect3D get() override;}
			virtual property bool IsClosed{bool get(); }
			virtual property double Volume{double get(); }
			virtual property double SurfaceArea { double get(); }
			virtual property bool IsPolyhedron { bool get(); }
			virtual property bool HasValidTopology{bool get(); }
			virtual IXbimSolidSet^ Cut(IXbimSolid^ toCut, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Union(IXbimSolid^ toUnion, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Intersection(IXbimSolid^ toIntersect, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Cut(IXbimSolidSet^ toCut, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Union(IXbimSolidSet^ toUnion, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Intersection(IXbimSolidSet^ toIntersect, double tolerance, ILogger^ logger);
			virtual IXbimFaceSet^ Section(IXbimFace^ face, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual void SaveAsBrep(String^ fileName);
#pragma endregion

#pragma region destructors

			~XbimSolid(){ InstanceCleanup(); }
			!XbimSolid(){ InstanceCleanup(); }

#pragma endregion

#pragma region constructors
			XbimSolid(){};
			XbimSolid(const TopoDS_Solid& solid);
			XbimSolid(const TopoDS_Solid& solid, Object^ tag);
			XbimSolid(IIfcSolidModel^ solid, ILogger^ logger);
			XbimSolid(IIfcManifoldSolidBrep^ solid, ILogger^ logger);
			XbimSolid(IIfcSweptAreaSolid^ solid, ILogger^ logger);
			XbimSolid(IIfcSweptAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger);
			XbimSolid(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolid(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^ logger);
			XbimSolid(IIfcHalfSpaceSolid^ solid, double maxExtrusion, XbimPoint3D centroid, ILogger^ logger);
			XbimSolid(IIfcHalfSpaceSolid^ solid,  ILogger^ logger);
			XbimSolid(IIfcBoxedHalfSpace^ solid, ILogger^ logger);
			XbimSolid(IIfcPolygonalBoundedHalfSpace^ solid, double maxExtrusion, ILogger^ logger);
			XbimSolid(IIfcExtrudedAreaSolid^ solid, ILogger^ logger);
			XbimSolid(IIfcExtrudedAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolid(IIfcExtrudedAreaSolidTapered^ solid, ILogger^ logger);
			XbimSolid(IIfcExtrudedAreaSolidTapered^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolid(IIfcRevolvedAreaSolidTapered^ solid, ILogger^ logger);
			XbimSolid(IIfcRevolvedAreaSolidTapered^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolid(IIfcRevolvedAreaSolid^ solid, ILogger^ logger);
			XbimSolid(IIfcRevolvedAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolid(IIfcSweptDiskSolid^ solid, ILogger^ logger);
			XbimSolid(IIfcSweptDiskSolidPolygonal^ solid, ILogger^ logger);
			XbimSolid(IIfcSectionedSpine^ solid, ILogger^ logger);
			XbimSolid(IIfcBoundingBox^ solid, ILogger^ logger);
			
			
			XbimSolid(IIfcFixedReferenceSweptAreaSolid^ solid, ILogger^ logger);
			XbimSolid(IIfcFixedReferenceSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger); //support for composite profiles
			XbimSolid(IIfcCsgPrimitive3D^ IIfcSolid, ILogger^ logger);
			
			XbimSolid(IIfcSphere^ IIfcSolid, ILogger^ logger);
			XbimSolid(IIfcBlock^ IIfcSolid, ILogger^ logger);
			XbimSolid(IIfcRightCircularCylinder^ IIfcSolid, ILogger^ logger);
			XbimSolid(IIfcRightCircularCone^ IIfcSolid, ILogger^ logger);
			XbimSolid(IIfcRectangularPyramid^ IIfcSolid, ILogger^ logger);
			XbimSolid(XbimRect3D rect3D, double tolerance, ILogger^ logger);
			XbimSolid(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger);
			XbimSolid(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger);
			XbimSolid(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger);
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
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;


			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;

			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger) override;
			virtual void Move(TopLoc_Location loc);
};

	}
}