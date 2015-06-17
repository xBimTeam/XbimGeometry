#include "XbimVertex.h"
#include "XbimGeomPrim.h"
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <TopoDS.hxx>
using namespace System;
namespace Xbim
{
	namespace Geometry
	{	
		
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimVertex::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (TopoDS_Vertex*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}


#pragma region Constructors
		/*Constructs an topological vertex with no geometry*/
		XbimVertex::XbimVertex()
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			b.MakeVertex(*pVertex);
		};



		XbimVertex::XbimVertex(XbimPoint3D point3D, double precision)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(point3D.X, point3D.Y, point3D.Z);
			b.MakeVertex(*pVertex, pnt, precision);
		}

		XbimVertex::XbimVertex(const TopoDS_Vertex& occVertex)
		{
			pVertex = new TopoDS_Vertex();
			*pVertex = occVertex;
		}

		XbimVertex::XbimVertex(IXbimVertex^ vertex, double precision)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(vertex->VertexGeometry.X, vertex->VertexGeometry.Y, vertex->VertexGeometry.Z);
			b.MakeVertex(*pVertex, pnt, precision);
		}

		XbimRect3D XbimVertex::BoundingBox::get()
		{	
			if (!IsValid) return XbimRect3D::Empty;
			return XbimRect3D(VertexGeometry.X, VertexGeometry.Y, VertexGeometry.Z, 0, 0, 0);
		}

		IXbimGeometryObject^ XbimVertex::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimGeomPrim::ToTransform(matrix3D));
			TopoDS_Vertex temp = TopoDS::Vertex(gTran.Shape());
			return gcnew XbimVertex(temp);
		}
		
#ifdef USE_CARVE_CSG
		XbimVertex::XbimVertex(vertex_t* v, double precision)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(v->v.x, v->v.y, v->v.z);
			b.MakeVertex(*pVertex, pnt, precision);
		}
#endif // USE_CARVE_CSG

#pragma endregion


#pragma region Equality Overrides

		bool XbimVertex::Equals(Object^ obj)
		{
			XbimVertex^ v = dynamic_cast< XbimVertex^>(obj);
			// Check for null
			if (v == nullptr) return false;
			return this == v;
		}

		bool XbimVertex::Equals(IXbimVertex^ obj)
		{
			XbimVertex^ v = dynamic_cast< XbimVertex^>(obj);
			// Check for null
			if (v == nullptr) return false;
			return this == v;
		}

		int XbimVertex::GetHashCode()
		{
			if (!IsValid) return 0;
			double tolerance = BRep_Tool::Tolerance(*pVertex);
			double gridDim = tolerance * 10.; //coursen  up
			//This hashcode snaps points to a grid of 100 * tolerance to ensure similar points fall into the same hash cell
			double xs = VertexGeometry.X - std::fmod(VertexGeometry.X, gridDim);
			double ys = VertexGeometry.Y - std::fmod(VertexGeometry.Y, gridDim);
			double zs = VertexGeometry.Z - std::fmod(VertexGeometry.Z, gridDim);
			int hash = (int)2166136261;
			hash = hash * 16777619 ^ xs.GetHashCode();
			hash = hash * 16777619 ^ ys.GetHashCode();
			hash = hash * 16777619 ^ zs.GetHashCode();
			return hash;
		}

		bool XbimVertex::operator ==(XbimVertex^ left, XbimVertex^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			//for vertex we use IsSame for equlaity as we do not care about orientation
			return  ((const TopoDS_Vertex&)left).IsSame(right) == Standard_True;

		}

		bool XbimVertex::operator !=(XbimVertex^ left, XbimVertex^ right)
		{
			return !(left == right);
		}


#pragma endregion



		XbimPoint3D XbimVertex::VertexGeometry::get()
		{
			if (!IsValid) return XbimPoint3D();
			gp_Pnt p = BRep_Tool::Pnt(*pVertex);
			return XbimPoint3D(p.X(), p.Y(), p.Z());
		}
	}
}