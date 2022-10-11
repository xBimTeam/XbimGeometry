
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <TopoDS.hxx>
#include "XbimVertex.h"
#include "XbimConvert.h"
namespace Xbim
{
	namespace Geometry
	{	
		
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimVertexV5::InstanceCleanup()
		{
			System::IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, System::IntPtr::Zero);
			if (temp != System::IntPtr::Zero)
				delete (TopoDS_Vertex*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}


#pragma region Constructors
		/*Constructs an topological vertex with no geometry*/
		XbimVertexV5::XbimVertexV5()
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			b.MakeVertex(*pVertex);
		};

		XbimVertexV5::XbimVertexV5(const TopoDS_Vertex& vertex, Object^ tag) :XbimVertexV5(vertex)
		{
			Tag = tag;
		}

		XbimVertexV5::XbimVertexV5(IIfcCartesianPoint^ vertex)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(vertex->X, vertex->Y, vertex->Z);
			b.MakeVertex(*pVertex, pnt, vertex->Model->ModelFactors->Precision);
		}

		XbimVertexV5::XbimVertexV5(double x, double y, double z, double precision)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(x, y, z);
			b.MakeVertex(*pVertex, pnt, precision);
		}
		XbimVertexV5::XbimVertexV5(XbimPoint3DWithTolerance^ point3D)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(point3D->X, point3D->Y, point3D->Z);
			b.MakeVertex(*pVertex, pnt, point3D->Tolerance);
		}

		XbimVertexV5::XbimVertexV5(XbimPoint3D point3D, double precision)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(point3D.X, point3D.Y, point3D.Z);
			b.MakeVertex(*pVertex, pnt, precision);
		}

		XbimVertexV5::XbimVertexV5(const TopoDS_Vertex& occVertex)
		{
			pVertex = new TopoDS_Vertex();
			*pVertex = occVertex;
		}

		XbimVertexV5::XbimVertexV5(IXbimVertex^ vertex, double precision)
		{
			pVertex = new TopoDS_Vertex();
			BRep_Builder b;
			gp_Pnt pnt(vertex->VertexGeometry.X, vertex->VertexGeometry.Y, vertex->VertexGeometry.Z);
			b.MakeVertex(*pVertex, pnt, precision);
		}

		XbimRect3D XbimVertexV5::BoundingBox::get()
		{	
			if (!IsValid) return XbimRect3D::Empty;
			return XbimRect3D(VertexGeometry.X, VertexGeometry.Y, VertexGeometry.Z, 0, 0, 0);
		}

		IXbimGeometryObject^ XbimVertexV5::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimConvert::ToTransform(matrix3D));
			TopoDS_Vertex temp = TopoDS::Vertex(gTran.Shape());
			return gcnew XbimVertexV5(temp);
		}
		
		IXbimGeometryObject^ XbimVertexV5::TransformShallow(XbimMatrix3D matrix3D)
		{
			TopoDS_Vertex vertex = TopoDS::Vertex(pVertex->Moved(XbimConvert::ToTransform(matrix3D)));
			System::GC::KeepAlive(this);
			return gcnew XbimVertexV5(vertex);
		}


		XbimGeometryObject ^ XbimVertexV5::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);			
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(this, trans, Standard_True); //make a copy of underlying shape
				return gcnew XbimVertexV5(TopoDS::Vertex(tr.Shape()), Tag);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(this, trans, Standard_False); //do not make a copy of underlying shape
				return gcnew XbimVertexV5(TopoDS::Vertex(tr.Shape()), Tag);
			}
		}
		void XbimVertexV5::Move(TopLoc_Location loc)
		{
			if (IsValid) pVertex->Move(loc);
		}
		XbimGeometryObject ^ XbimVertexV5::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimVertexV5^ copy = gcnew XbimVertexV5(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject ^ XbimVertexV5::Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;			
			XbimVertexV5^ copy = gcnew XbimVertexV5(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement,logger);
			copy->Move(loc);
			return copy;
		}

		void XbimVertexV5::Mesh(IXbimMeshReceiver ^ /*mesh*/, double /*precision*/ , double /*deflection*/ , double /*angle*/  )
		{
			throw gcnew System::NotImplementedException("XbimVertex::Mesh");
		}

#pragma endregion


#pragma region Equality Overrides

		bool XbimVertexV5::Equals(Object^ obj)
		{
			XbimVertexV5^ v = dynamic_cast< XbimVertexV5^>(obj);
			// Check for null
			if (v == nullptr) return false;
			return this == v;
		}

		bool XbimVertexV5::Equals(IXbimVertex^ obj)
		{
			XbimVertexV5^ v = dynamic_cast< XbimVertexV5^>(obj);
			// Check for null
			if (v == nullptr) return false;
			return this == v;
		}

		int XbimVertexV5::GetHashCode()
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

		bool XbimVertexV5::operator ==(XbimVertexV5^ left, XbimVertexV5^ right)
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

		bool XbimVertexV5::operator !=(XbimVertexV5^ left, XbimVertexV5^ right)
		{
			return !(left == right);
		}


#pragma endregion



		XbimPoint3D XbimVertexV5::VertexGeometry::get()
		{
			if (!IsValid) return XbimPoint3D();
			gp_Pnt p = BRep_Tool::Pnt(*pVertex);
			System::GC::KeepAlive(this);
			return XbimPoint3D(p.X(), p.Y(), p.Z());
		}
	}
}