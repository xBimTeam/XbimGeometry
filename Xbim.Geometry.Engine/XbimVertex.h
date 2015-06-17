#pragma once
#include "XbimOccShape.h"
#include <TopoDS_Vertex.hxx>
#include "CarveDeclarations.h"

using namespace XbimGeometry::Interfaces;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimVertex : IXbimVertex, XbimOccShape
		{
		protected:
			IntPtr ptrContainer;
			virtual property TopoDS_Vertex* pVertex
			{
				TopoDS_Vertex* get() sealed { return (TopoDS_Vertex*)ptrContainer.ToPointer(); }
				void set(TopoDS_Vertex* val)sealed { ptrContainer = IntPtr(val); }
			}
			void InstanceCleanup();
		public:

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimVertex^ left, XbimVertex^ right);
			static bool operator !=(XbimVertex^ left, XbimVertex^ right);
			virtual bool Equals(IXbimVertex^ v);
#pragma endregion



#pragma region IXbimVertexInterfaces
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimVertexType; }; }
			virtual property bool IsValid{bool get() override { return pVertex != nullptr; }; }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
			virtual property XbimPoint3D VertexGeometry {XbimPoint3D get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
#pragma endregion


			
#pragma region Constructors
			///Constructs vertex with no geometric location
			XbimVertex();
			XbimVertex(XbimPoint3D point3D, double precision);
			XbimVertex(const TopoDS_Vertex& occVertex);
			XbimVertex(IXbimVertex^ vertex, double precision);

#ifdef USE_CARVE_CSG
			XbimVertex(vertex_t* v, double precision);
#endif // USE_CARVE_CSG

#pragma endregion

#pragma region Destructors

			~XbimVertex(){ InstanceCleanup(); }
			!XbimVertex(){ InstanceCleanup(); }

#pragma endregion


#pragma region Operators

			operator const TopoDS_Vertex& () { return *pVertex; }
			virtual operator const TopoDS_Shape& () override { return *pVertex; }

#pragma endregion

		};
	}
}
