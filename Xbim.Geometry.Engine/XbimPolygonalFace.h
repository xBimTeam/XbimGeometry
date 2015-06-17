#pragma once
#ifdef USE_CARVE_CSG

#include "XbimFacetedSolid.h"


using namespace XbimGeometry::Interfaces;

namespace Xbim
{
	namespace Geometry
	{
		ref class XbimPolygonalFace : IXbimFace
		{
		private:
			XbimFacetedSolid^ facetedSolid; //needed to stop garbage collector cleaning up
			face_t* pFace; //nb we do not delete this on destruction it is managed by the facetedSolid life

			//helpers
			void ProcessBound(edge_t* start, List<IXbimWire^>^ wires);
		public:
			XbimPolygonalFace(XbimFacetedSolid^ facetedSolid, face_t* face);

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimPolygonalFace^ left, XbimPolygonalFace^ right);
			static bool operator !=(XbimPolygonalFace^ left, XbimPolygonalFace^ right);
			virtual bool Equals(IXbimFace^ f);
#pragma endregion


#pragma region IXbimFace Interface
			virtual property bool IsValid{bool get() { return pFace != nullptr; }; }
			virtual property bool IsSet{bool get()  { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimFaceType; }; }
			virtual property IXbimWire^ OuterBound{ IXbimWire^ get(); }
			virtual property IXbimWireSet^ InnerBounds{IXbimWireSet^ get(); }
			virtual property double Area { double get(); }
			virtual property double Perimeter { double get(); }
			virtual property bool IsPlanar{bool get(); }
			virtual property XbimRect3D BoundingBox{XbimRect3D get(); }
			///The topological normal of the face
			virtual property XbimVector3D Normal{ XbimVector3D get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D);
#pragma endregion

#pragma region Operators
			operator const face_t* () { return pFace; }
			operator const face_t& () { return *pFace; }
#pragma endregion


		};
	}
}

#endif // DEBUG
