#pragma once
#include "XbimVertex.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
using namespace XbimGeometry::Interfaces;
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimVertexSet : IXbimVertexSet
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

#pragma region IXbimVertexSet Interface definition
			virtual property bool IsValid{bool get(){ return Count>0; }; }
			virtual property bool IsSet{bool get() { return true; }; }
			virtual property IXbimVertex^ First{IXbimVertex^ get(); }
			virtual property int Count{int get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get(); }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimVertexSetType; }}
			virtual IEnumerator<IXbimVertex^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
#pragma endregion

		};

	}
}

