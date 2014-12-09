#pragma once
#include "XbimEdge.h"
#include "XbimWire.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
using namespace XbimGeometry::Interfaces;
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimEdgeSet : IXbimEdgeSet
		{
		private:			
			List<IXbimEdge^>^ edges;
			static XbimEdgeSet^ empty = gcnew XbimEdgeSet();
			XbimEdgeSet::XbimEdgeSet(){ edges = gcnew List<IXbimEdge^>(1); }

		public:
			static property XbimEdgeSet^ Empty{XbimEdgeSet^ get(){ return empty; }};

#pragma region Constructors
			XbimEdgeSet(const TopoDS_Shape& shape);
			XbimEdgeSet(IEnumerable<IXbimEdge^>^ edges);
			XbimEdgeSet(XbimWire^ wire);
			
#pragma endregion


#pragma region IXbimEdgeSet Interface
			virtual property bool IsValid{bool get(){ return Count>0; }; }
			virtual property bool IsSet{bool get() { return true; }; }
			virtual property IXbimEdge^ First{IXbimEdge^ get(); }
			virtual property int Count{int get(); }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimEdgeSetType; }}
			virtual IEnumerator<IXbimEdge^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() ; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
#pragma endregion
		};
		
	}
}

