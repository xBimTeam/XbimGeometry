#pragma once
#include "XbimOccShape.h"
#include "XbimWire.h"
#include "XbimFace.h"
#include "XbimFaceSet.h"
#include <TopoDS_Solid.hxx>

using namespace System::Collections::Generic;
using namespace System::IO;
using namespace Xbim::Ifc4;
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

			void Init(IIfcSolidModel^ solid);
			void Init(IIfcManifoldSolidBrep^ solid);
			void Init(IIfcSweptAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef);
			void Init(IIfcExtrudedAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef);
			void Init(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef);
			void Init(IIfcRevolvedAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef);

			void Init(IIfcExtrudedAreaSolidTapered^ solid, IIfcProfileDef^ overrideProfileDef);
			void Init(IIfcRevolvedAreaSolidTapered^ solid, IIfcProfileDef^ overrideProfileDef);
			void Init(IIfcSectionedSpine^ solid);
			void Init(IIfcSweptDiskSolid^ solid);
			void Init(IIfcSweptDiskSolidPolygonal^ solid);
			void Init(IIfcBoundingBox^ solid);
			void Init(IIfcHalfSpaceSolid^ solid, double maxExtrusion, XbimPoint3D centroid);
			void Init(IIfcBoxedHalfSpace^ solid);
			void Init(IIfcPolygonalBoundedHalfSpace^ solid, double maxExtrusion);
			void Init(IIfcBooleanResult^ solid);
			void Init(IIfcBooleanOperand^ solid);
			void Init(XbimRect3D rect3D, double tolerance);

			void Init(IIfcFixedReferenceSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef);
			void Init(IIfcCsgPrimitive3D^ IIfcSolid);
			void Init(IIfcCsgSolid^ IIfcSolid);
			void Init(IIfcSphere^ IIfcSolid);
			void Init(IIfcBlock^ IIfcSolid);
			void Init(IIfcRightCircularCylinder^ IIfcSolid);
			void Init(IIfcRightCircularCone^ IIfcSolid);
			void Init(IIfcRectangularPyramid^ IIfcSolid);
#pragma endregion

		public:
			static XbimSolid^ BuildClippingList(IIfcBooleanResult^ solid, List<IIfcBooleanOperand^>^ clipList);
#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimSolid^ left, XbimSolid^ right);
			static bool operator !=(XbimSolid^ left, XbimSolid^ right);
			virtual bool Equals(IXbimSolid^ s);
#pragma endregion

#pragma region IXbimSolid Interface
			virtual property bool IsValid{bool get() override { return pSolid != nullptr; }; }
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
			virtual IXbimSolidSet^ Cut(IXbimSolid^ toCut, double tolerance);
			virtual IXbimSolidSet^ Union(IXbimSolid^ toUnion, double tolerance);
			virtual IXbimSolidSet^ Intersection(IXbimSolid^ toIntersect, double tolerance);
			virtual IXbimSolidSet^ Cut(IXbimSolidSet^ toCut, double tolerance);
			virtual IXbimSolidSet^ Union(IXbimSolidSet^ toUnion, double tolerance);
			virtual IXbimSolidSet^ Intersection(IXbimSolidSet^ toIntersect, double tolerance);
			virtual IXbimFaceSet^ Section(IXbimFace^ face, double tolerance);
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
			XbimSolid(IIfcSolidModel^ solid);
			XbimSolid(IIfcManifoldSolidBrep^ solid);
			XbimSolid(IIfcSweptAreaSolid^ solid);
			XbimSolid(IIfcSweptAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef);
			XbimSolid(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef); //support for composite profiles
			XbimSolid(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid);
			XbimSolid(IIfcHalfSpaceSolid^ solid, double maxExtrusion, XbimPoint3D centroid);
			XbimSolid(IIfcBoxedHalfSpace^ solid);
			XbimSolid(IIfcPolygonalBoundedHalfSpace^ solid, double maxExtrusion);
			XbimSolid(IIfcExtrudedAreaSolid^ solid);
			XbimSolid(IIfcExtrudedAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef); //support for composite profiles
			XbimSolid(IIfcExtrudedAreaSolidTapered^ solid);
			XbimSolid(IIfcExtrudedAreaSolidTapered^ IIfcSolid, IIfcProfileDef^ overrideProfileDef); //support for composite profiles
			XbimSolid(IIfcRevolvedAreaSolidTapered^ solid);
			XbimSolid(IIfcRevolvedAreaSolidTapered^ IIfcSolid, IIfcProfileDef^ overrideProfileDef); //support for composite profiles
			XbimSolid(IIfcRevolvedAreaSolid^ solid);
			XbimSolid(IIfcRevolvedAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef); //support for composite profiles
			XbimSolid(IIfcSweptDiskSolid^ solid);
			XbimSolid(IIfcSweptDiskSolidPolygonal^ solid);
			XbimSolid(IIfcSectionedSpine^ solid);
			XbimSolid(IIfcBoundingBox^ solid);
			XbimSolid(IIfcBooleanResult^ solid);
			XbimSolid(IIfcBooleanOperand^ solid);
			XbimSolid(IIfcFixedReferenceSweptAreaSolid^ solid);
			XbimSolid(IIfcFixedReferenceSweptAreaSolid^ IIfcSolid, IIfcProfileDef^ overrideProfileDef); //support for composite profiles
			XbimSolid(IIfcCsgPrimitive3D^ IIfcSolid);
			XbimSolid(IIfcCsgSolid^ IIfcSolid);
			XbimSolid(IIfcSphere^ IIfcSolid);
			XbimSolid(IIfcBlock^ IIfcSolid);
			XbimSolid(IIfcRightCircularCylinder^ IIfcSolid);
			XbimSolid(IIfcRightCircularCone^ IIfcSolid);
			XbimSolid(IIfcRectangularPyramid^ IIfcSolid);
			XbimSolid(XbimRect3D rect3D, double tolerance);
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
			void FixTopology();
#pragma endregion

			

			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;


			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;

			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement) override;
			virtual void Move(TopLoc_Location loc);
};

	}
}