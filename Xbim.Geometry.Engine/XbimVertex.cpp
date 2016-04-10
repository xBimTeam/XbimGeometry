#include "XbimVertex.h"
#include "XbimConvert.h"
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
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

		XbimVertex::XbimVertex(const TopoDS_Vertex& vertex, Object^ tag) :XbimVertex(vertex)
		{
			Tag = tag;
		}

		XbimVertex::XbimVertex(IIfcCartesianPoint^ vertex)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(vertex->X, vertex->Y, vertex->Z);
			b.MakeVertex(*pVertex, pnt, vertex->Model->ModelFactors->Precision);
		}

		XbimVertex::XbimVertex(double x, double y, double z, double precision)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(x, y, z);
			b.MakeVertex(*pVertex, pnt, precision);
		}
		XbimVertex::XbimVertex(XbimPoint3DWithTolerance^ point3D)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(point3D->X, point3D->Y, point3D->Z);
			b.MakeVertex(*pVertex, pnt, point3D->Tolerance);
		}

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
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimConvert::ToTransform(matrix3D));
			TopoDS_Vertex temp = TopoDS::Vertex(gTran.Shape());
			return gcnew XbimVertex(temp);
		}
		
		IXbimGeometryObject^ XbimVertex::TransformShallow(XbimMatrix3D matrix3D)
		{
			TopoDS_Vertex vertex = TopoDS::Vertex(pVertex->Moved(XbimConvert::ToTransform(matrix3D)));
			GC::KeepAlive(this);			
			return gcnew XbimVertex(vertex);
		}


		XbimGeometryObject ^ XbimVertex::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);			
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(this, trans, Standard_True); //make a copy of underlying shape
				return gcnew XbimVertex(TopoDS::Vertex(tr.Shape()), Tag);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(this, trans, Standard_False); //do not make a copy of underlying shape
				return gcnew XbimVertex(TopoDS::Vertex(tr.Shape()), Tag);
			}
		}
		void XbimVertex::Move(TopLoc_Location loc)
		{
			if (IsValid) pVertex->Move(loc);
		}
		XbimGeometryObject ^ XbimVertex::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimVertex^ copy = gcnew XbimVertex(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject ^ XbimVertex::Moved(IIfcObjectPlacement ^ objectPlacement)
		{
			if (!IsValid) return this;			
			XbimVertex^ copy = gcnew XbimVertex(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement);
			copy->Move(loc);
			return copy;
		}

		void XbimVertex::Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle)
		{
			return;//maybe add an implementation for this
		}

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