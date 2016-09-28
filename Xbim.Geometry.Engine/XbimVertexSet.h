#pragma once
#include "XbimVertex.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimVertexSet : XbimSetObject, IXbimVertexSet
		{
		private:			
			List<IXbimVertex^>^ vertices;
			static XbimVertexSet^ empty = gcnew XbimVertexSet();
			XbimVertexSet::XbimVertexSet(){ vertices = gcnew List<IXbimVertex^>(1); }
			void InstanceCleanup()
			{
				vertices = nullptr;
			};
		public:
			static property XbimVertexSet^ Empty{XbimVertexSet^ get(){ return empty; }};

#pragma region Constructors
			XbimVertexSet(const TopoDS_Shape& shape);
			XbimVertexSet(IEnumerable<IXbimVertex^>^ vertices);
#pragma endregion

#pragma region destructors

			~XbimVertexSet(){ InstanceCleanup(); }
			!XbimVertexSet(){ InstanceCleanup(); }

#pragma endregion


#pragma region operators
			virtual property IXbimVertex^ default[int]
			{
				IXbimVertex^ get(int index)
				{
					return vertices[index];
				}

				void set(int index, IXbimVertex^ value)
				{
					vertices[index] = value;
				}
			}
			property XbimVertex^ Vertex[int]
			{
				XbimVertex^ get(int index)
				{
					return (XbimVertex^)vertices[index];
				}

				void set(int index, XbimVertex^ value)
				{
					vertices[index] = value;
				}
			}
#pragma endregion

#pragma region IXbimVertexSet Interface definition
			virtual property bool IsValid{bool get(){ return Count>0; }; }
			virtual property bool IsSet{bool get() { return true; }; }
			virtual void Add(IXbimVertex^ vertex) { vertices->Add(vertex); };
			virtual property IXbimVertex^ First{IXbimVertex^ get(); }
			virtual property int Count {int get() override; }
			virtual IXbimGeometryObject^ Trim()  override { if (Count == 1) return First; else if (Count == 0) return nullptr; else return this; };
			virtual property XbimRect3D BoundingBox {XbimRect3D get(); }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimVertexSetType; }}
			virtual IEnumerator<IXbimVertex^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);
#pragma endregion


			// Inherited via XbimSetObject
			virtual IXbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;


			// Inherited via XbimSetObject
			virtual IXbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;

			virtual IXbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement) override;


			// Inherited via XbimSetObject
			virtual void Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle) override;

		};

	}
}

