#pragma once
#include "XbimOccShape.h"
#include "XbimPoint3DWithTolerance.h"
#include <TopoDS_Vertex.hxx>
#include <BRep_Tool.hxx>
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
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
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
#pragma endregion


			
#pragma region Constructors
			///Constructs vertex with no geometric location
			XbimVertex();
			XbimVertex(XbimPoint3D point3D, double precision);
			XbimVertex(XbimPoint3DWithTolerance^ point3D);
			XbimVertex(const TopoDS_Vertex& occVertex);
			XbimVertex(const TopoDS_Vertex& occVertex, Object^ tag);
			XbimVertex(IXbimVertex^ vertex, double precision);
			XbimVertex(IIfcCartesianPoint^ vertex);
			XbimVertex(double x, double y, double z, double precision);
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
			property double Tolerance{double get(){ return IsValid ? BRep_Tool::Tolerance(*pVertex) : 0; }}
#pragma endregion


			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;


			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;
			virtual void Move(TopLoc_Location loc);

			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement) override;


			// Inherited via XbimOccShape
			virtual void Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle) override;

		};
	}
}
