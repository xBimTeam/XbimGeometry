#pragma once
#include "XbimOccShape.h"
#include "XbimWire.h"
#include "XbimFace.h"
#include "XbimFaceSet.h"
#include <TopoDS_Solid.hxx>

using namespace System::Collections::Generic;
using namespace System::IO;
using namespace XbimGeometry::Interfaces;
using namespace Xbim::Ifc2x3::GeometricModelResource;
using namespace Xbim::Ifc2x3::GeometryResource;
using namespace Xbim::Common::Geometry;
using namespace Xbim::XbimExtensions::SelectTypes;
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

			void Init(IfcSolidModel^ solid);
			void Init(IfcManifoldSolidBrep^ solid);
			void Init(IfcSweptAreaSolid^ solid);
			void Init(IfcExtrudedAreaSolid^ solid);
			void Init(IfcSurfaceCurveSweptAreaSolid^ ifcSolid);

			void Init(IfcRevolvedAreaSolid^ solid);
			void Init(IfcSweptDiskSolid^ solid);
			void Init(IfcBoundingBox^ solid);
			void Init(IfcHalfSpaceSolid^ solid, double maxExtrusion);
			void Init(IfcBoxedHalfSpace^ solid);
			void Init(IfcPolygonalBoundedHalfSpace^ solid, double maxExtrusion);
			void Init(IfcBooleanResult^ solid);
			void Init(IfcBooleanOperand^ solid);
			void Init(XbimRect3D rect3D, double tolerance);


			void Init(IfcCsgPrimitive3D^ ifcSolid);
			void Init(IfcCsgSolid^ ifcSolid);
			void Init(IfcSphere^ ifcSolid);
			void Init(IfcBlock^ ifcSolid);
			void Init(IfcRightCircularCylinder^ ifcSolid);
			void Init(IfcRightCircularCone^ ifcSolid);
			void Init(IfcRectangularPyramid^ ifcSolid);
#pragma endregion

		public:
			static XbimSolid^ BuildClippingList(IfcBooleanResult^ solid, List<IfcBooleanOperand^>^ clipList);
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
#pragma endregion

#pragma region destructors

			~XbimSolid(){ InstanceCleanup(); }
			!XbimSolid(){ InstanceCleanup(); }

#pragma endregion

#pragma region constructors
			XbimSolid(){};
			XbimSolid(const TopoDS_Solid& solid);
			XbimSolid(IfcSolidModel^ solid);
			XbimSolid(IfcManifoldSolidBrep^ solid);
			XbimSolid(IfcSweptAreaSolid^ solid);
			XbimSolid(IfcSurfaceCurveSweptAreaSolid^ ifcSolid);
			XbimSolid(IfcHalfSpaceSolid^ solid, double maxExtrusion);
			XbimSolid(IfcBoxedHalfSpace^ solid);
			XbimSolid(IfcPolygonalBoundedHalfSpace^ solid, double maxExtrusion);
			XbimSolid(IfcExtrudedAreaSolid^ solid);
			XbimSolid(IfcRevolvedAreaSolid^ solid);
			XbimSolid(IfcSweptDiskSolid^ solid);
			XbimSolid(IfcBoundingBox^ solid);
			XbimSolid(IfcBooleanResult^ solid);
			XbimSolid(IfcBooleanOperand^ solid);

			XbimSolid(IfcCsgPrimitive3D^ ifcSolid);
			XbimSolid(IfcCsgSolid^ ifcSolid);
			XbimSolid(IfcSphere^ ifcSolid);
			XbimSolid(IfcBlock^ ifcSolid);
			XbimSolid(IfcRightCircularCylinder^ ifcSolid);
			XbimSolid(IfcRightCircularCone^ ifcSolid);
			XbimSolid(IfcRectangularPyramid^ ifcSolid);
			XbimSolid(XbimRect3D rect3D, double tolerance);
#pragma endregion

			
#pragma region operators
			operator const TopoDS_Solid& () { return *pSolid; }
			virtual operator const TopoDS_Shape& () override { return *pSolid; }
#pragma endregion

		
		
#pragma region Methods
			//moves the solid to the new position
			void Move(IfcAxis2Placement3D^ position);
			void Translate(XbimVector3D translation);
			void Reverse();
			
			void FixTopology();
#pragma endregion

			
		};

	}
}