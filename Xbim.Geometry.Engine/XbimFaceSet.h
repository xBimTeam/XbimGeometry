#pragma once
#include "XbimFace.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_ListOfShape.hxx>

using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimFaceSet : XbimSetObject, IXbimFaceSet
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


#pragma region operators

			virtual property IXbimFace^ default[int]
			{
				IXbimFace^ get(int index)
				{
					return faces[index];
				}

				void set(int index, IXbimFace^ value)
				{
					faces[index] = value;
				}
			}
			property XbimFace^ Face[int]
			{
				XbimFace^ get(int index)
				{
					return (XbimFace^)faces[index];
				}

				void set(int index, XbimFace^ value)
				{
					faces[index] = value;
				}
			}
#pragma endregion

#pragma region IXbimFaceSet Interface

			virtual property bool IsValid{bool get(){ return true; }; }
			virtual property bool IsSet{bool get()  { return true; }; }
			virtual void Add(IXbimFace^ face) { faces->Add(face); };
			virtual property IXbimFace^ First{IXbimFace^ get(); }
			virtual property int Count {int get() override; }
			virtual IXbimGeometryObject^ Trim()  override { if (Count == 1) return First; else if (Count == 0) return nullptr; else return this; };
			virtual property XbimRect3D BoundingBox {XbimRect3D get(); }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimFaceSetType; }}
			virtual IEnumerator<IXbimFace^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D);
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

