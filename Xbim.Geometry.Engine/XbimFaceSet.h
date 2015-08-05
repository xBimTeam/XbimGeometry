#pragma once
#include "XbimFace.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_ListOfShape.hxx>

using namespace XbimGeometry::Interfaces;
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimFaceSet : IXbimFaceSet
		{
		private:
			List<IXbimFace^>^ faces;
			static XbimFaceSet^ empty = gcnew XbimFaceSet();
			XbimFaceSet::XbimFaceSet(){ faces = gcnew List<IXbimFace^>(1); }
			void InstanceCleanup()
			{
				faces = nullptr;
			};
		public:
			static property XbimFaceSet^ Empty{XbimFaceSet^ get(){ return empty; }};

#pragma region Constructors
			XbimFaceSet(const TopoDS_Shape& shape);
			XbimFaceSet(const TopTools_ListOfShape & shapes);
			XbimFaceSet(List<IXbimFace^>^ faces);
#pragma endregion

#pragma region destructors

			~XbimFaceSet(){ InstanceCleanup(); }
			!XbimFaceSet(){ InstanceCleanup(); }

#pragma endregion

#pragma region IXbimFaceSet Interface

			virtual property bool IsValid{bool get(){ return true; }; }
			virtual property bool IsSet{bool get()  { return true; }; }
			virtual property IXbimFace^ First{IXbimFace^ get(); }
			virtual property int Count{int get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get(); }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimFaceSetType; }}
			virtual IEnumerator<IXbimFace^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D);
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);
#pragma endregion

		};

	}
}

