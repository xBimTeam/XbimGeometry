#pragma once

#ifdef USE_CARVE_CSG 
#include "XbimSolid.h"
#include "XbimShell.h"

#include "CarveDeclarations.h"



using namespace XbimGeometry::Interfaces;

namespace Xbim
{
	namespace Geometry
	{
		ref class XbimFacetedSolid : IXbimSolid
		{
			IntPtr pMeshSet;
			double tolerance;
			void Init(XbimSolid^ solid, double tolerance, double deflection, double angle, bool triangulate);
			void InitPolyhedron(XbimSolid^ solid, double tolerance);
			//void Init(IXbimSolidSet^ solidSet, double tolerance, double deflection, double angle, bool triangulate);
			void Init(IfcBooleanClippingResult^ clip);
			void InstanceCleanup();
			XbimFacetedSolid^ MakeInfiniteFace(XbimPoint3D l, XbimVector3D n);
			static void  GetConnected(HashSet<XbimFacetedSolid^>^ connected, Dictionary<XbimFacetedSolid^, HashSet<XbimFacetedSolid^>^>^ clusters, XbimFacetedSolid^ clusterAround);
		protected:
			///Returns the pointer to the facet mesh data, it is the responsibility of the caller to delete this when not required
			///This faceted solid is no longer valid after this call;
			IntPtr Nullify();
		public:
#pragma region Constructors
			XbimFacetedSolid();
			XbimFacetedSolid(meshset_t* mesh);
			//Converts an OCC type solid to a faceted solid
			XbimFacetedSolid(XbimSolid^ solid, double tolerance/*, double deflection=1.0, double angle=0.5, bool trangulate = false*/);
			XbimFacetedSolid(XbimSolid^ solid, double tolerance, double deflection, double angle/*, bool trangulate = false*/);
			XbimFacetedSolid(XbimSolid^ solid, double tolerance, double deflection/*, bool trangulate = false*/);

			XbimFacetedSolid(IXbimSolid^ solid, double tolerance, double deflection/*, bool trangulate = false*/);
			XbimFacetedSolid(IXbimSolid^ solid, double tolerance, double deflection, double angle/*, bool trangulate = false*/);
			XbimFacetedSolid(IXbimSolid^ solid, double tolerance, double deflection, double angle, bool trangulate);
			XbimFacetedSolid(IXbimSolid^ solid, double tolerance, double deflection, bool trangulate);

			XbimFacetedSolid(XbimShell^ shell);
			XbimFacetedSolid(XbimFace^ face);
			XbimFacetedSolid(IfcBooleanClippingResult^ ifcSolid);
#pragma endregion
#pragma region IXbimSolidSet constructors

			/*XbimFacetedSolid(IXbimSolidSet^ solidSet, double tolerance, double deflection);
			XbimFacetedSolid(IXbimSolidSet^ solidSet, double tolerance, double deflection, double angle);
			XbimFacetedSolid(IXbimSolidSet^ solidSet, double tolerance, double deflection, bool triangulate);
			XbimFacetedSolid(IXbimSolidSet^ solidSet, double tolerance, double deflection, double angle, bool triangulate);*/

#pragma endregion

#pragma region destructors
			~XbimFacetedSolid(){ InstanceCleanup(); }
			!XbimFacetedSolid(){ InstanceCleanup(); }
#pragma endregion


#pragma region IXbimSolid Interface		
			virtual property bool IsValid{bool get() { return pMeshSet!= IntPtr::Zero; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimMeshType; }; }
			virtual property IXbimShellSet^ Shells{ IXbimShellSet^ get(); }
			virtual property IXbimFaceSet^ Faces{ IXbimFaceSet^ get(); }
			virtual property IXbimEdgeSet^ Edges{ IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices{IXbimVertexSet^ get(); }
			virtual property XbimRect3D BoundingBox{XbimRect3D get(); }
			virtual property bool IsClosed{bool get(); }
			virtual property bool IsSet{bool get() { return false; }; }
			virtual property double Volume{double get(); }
			virtual property double SurfaceArea { double get(); }
			virtual property bool IsPolyhedron { bool get(); }
			virtual IXbimSolidSet^ Cut(IXbimSolid^ toCut, double tolerance);
			virtual IXbimSolidSet^ Union(IXbimSolid^ toUnion, double tolerance);
			virtual IXbimSolidSet^ Intersection(IXbimSolid^ toIntersect, double tolerance);
			virtual IXbimSolidSet^ Cut(IXbimSolidSet^ toCut, double tolerance);
			virtual IXbimSolidSet^ Union(IXbimSolidSet^ toUnion, double tolerance);
			virtual IXbimSolidSet^ Intersection(IXbimSolidSet^ toIntersect, double tolerance);
			virtual IXbimFaceSet^ Section(IXbimFace^ face, double tolerance);
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D);
#pragma endregion

#pragma region Properties
			virtual property double Tolerance{double get(){ return tolerance; }; }
#pragma endregion


#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimFacetedSolid^ left, XbimFacetedSolid^ right);
			static bool operator !=(XbimFacetedSolid^ left, XbimFacetedSolid^ right);
			virtual bool Equals(IXbimSolid^ s);
#pragma endregion

#pragma region Operators

			
			operator meshset_t* () { return (meshset_t*)pMeshSet.ToPointer(); }
			operator const meshset_t& () { return *(meshset_t*)pMeshSet.ToPointer(); }
#pragma endregion

#pragma region Methods
			IXbimSolid^ ConvertToXbimSolid();
			void WriteTriangulation(TextWriter^ textWriter, double tolerance, double deflection, double angle);
			void WriteTriangulation(BinaryWriter^ binaryWriter, double tolerance, double deflection, double angle);
			//merges coplanar faces, where the angle betweeen the normals is less than angle (radians)
			int MergeCoPlanarFaces(double normalAngle);
			static XbimFacetedSolid^ Merge(IXbimSolidSet^ facetedSolids, double tolerance);
#pragma endregion

		};
	}
}


#endif // USE_CARVE_CSG
