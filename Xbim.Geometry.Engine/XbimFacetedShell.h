#pragma once

#ifdef USE_CARVE_CSG
#include "XbimFacetedSolid.h"
 
using namespace XbimGeometry::Interfaces;

namespace Xbim
{
	namespace Geometry
	{
		ref class XbimFacetedShell :IXbimShell
		{
		private:
			XbimFacetedSolid^ facetedSolid; //needed to stop garbage collector cleaning up
			mesh_t* pMesh; //nb we do not delete this on destruction it is managed by the facetedSolid life
		public:
			XbimFacetedShell(XbimFacetedSolid^ fsolid, mesh_t* mesh);

#pragma region IXbimShell Interface		
			virtual property bool IsValid{bool get() { return pMesh != nullptr; }; }
			virtual property bool IsSet{bool get() { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimMeshType; }; }
			virtual property IXbimFaceSet^ Faces{ IXbimFaceSet^ get(); }
			virtual property IXbimEdgeSet^ Edges{ IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices{IXbimVertexSet^ get(); }
			virtual property XbimRect3D BoundingBox{XbimRect3D get(); }
			virtual property bool IsClosed{bool get(); }
			virtual property double Volume{double get(); }
			virtual property double SurfaceArea { double get(); }
			virtual property bool IsPolyhedron { bool get(); }
			virtual IXbimGeometryObject^ Cut(IXbimGeometryObject^ toCut, double tolerance);
			virtual IXbimGeometryObject^ Union(IXbimGeometryObject^ toUnion, double tolerance);
			virtual IXbimGeometryObject^ Intersection(IXbimGeometryObject^ toIntersect, double tolerance);
			virtual IXbimFaceSet^ Section(IXbimFace^ face, double tolerance);
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D);
#pragma endregion

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimFacetedShell^ left, XbimFacetedShell^ right);
			static bool operator !=(XbimFacetedShell^ left, XbimFacetedShell^ right);
			virtual bool Equals(IXbimShell^ s);
#pragma endregion

#pragma region Operators
			operator const mesh_t* () { return pMesh; }
			operator const mesh_t& () { return *pMesh; }
#pragma endregion

		};
	}
}
#endif // USE_CARVE_CSG


