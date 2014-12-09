#pragma once
#include "XbimGeometryObject.h"
using namespace XbimGeometry::Interfaces;
using namespace System::Collections::Generic;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{


		ref class XbimGeometryObjectSet : IXbimGeometryObjectSet
		{

		private:
			List<IXbimGeometryObject^>^ geometryObjects;
			static XbimGeometryObjectSet^ empty = gcnew XbimGeometryObjectSet();
			XbimGeometryObjectSet::XbimGeometryObjectSet(){ geometryObjects = gcnew List<IXbimGeometryObject^>(1); }
		public:
			static property IXbimGeometryObjectSet^ Empty{IXbimGeometryObjectSet^ get(){ return empty; }};		
			XbimGeometryObjectSet(IEnumerable<IXbimGeometryObject^>^ objects);
			XbimGeometryObjectSet(int size){geometryObjects = gcnew List<IXbimGeometryObject^>(size);}
#pragma region IXbimGeometryObjectSet Interface
			virtual property bool IsValid{bool get(){ return Count>0; }; }
			virtual property bool IsSet{bool get(){ return true; }; }
			virtual property IXbimGeometryObject^ First{IXbimGeometryObject^ get(); }
			virtual property int Count{int get(); }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimGeometryObjectSetType; }}
			virtual IEnumerator<IXbimGeometryObject^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() ; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
#pragma endregion
			void Add(IXbimGeometryObject^ geomObj){ geometryObjects->Add(geomObj); }
		};
	}
}
