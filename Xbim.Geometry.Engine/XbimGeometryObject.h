#pragma once
using namespace System;
using namespace Xbim::Common::Geometry;
using namespace System::Collections::Generic;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Ifc4;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimGeometryObjectEnumerator :IEnumerator<IXbimGeometryObject^>
		{
		private:
			IXbimGeometryObject^ shape;
			bool atStart;
		public:

			XbimGeometryObjectEnumerator(IXbimGeometryObject^ occShape)
			{
				shape = occShape;
				atStart = true;
			};

			virtual ~XbimGeometryObjectEnumerator()
			{
			}

			!XbimGeometryObjectEnumerator()
			{
			}

			virtual bool MoveNext(void) //there is only ever one so any move means we are at the end
			{
				atStart = false;
				return atStart;
			}

			virtual property IXbimGeometryObject^ Current
			{
				IXbimGeometryObject^ get()
				{
					return shape;
				}
			};
			// This is required as IEnumerator<T> also implements IEnumerator
			virtual property Object^ Current2
			{
				virtual Object^ get() sealed = System::Collections::IEnumerator::Current::get
				{
					return shape;
				}
			};

			virtual void Reset()
			{
				atStart = true;
			}
		};

		ref class XbimSetObject abstract
		{
		private:
			Object^ tag;
		public:
			virtual IXbimGeometryObject^ Transformed(IIfcCartesianTransformationOperator ^transformation) abstract;
			virtual IXbimGeometryObject^ Moved(IIfcPlacement ^placement) abstract;
			virtual IXbimGeometryObject^ Moved(IIfcObjectPlacement ^objectPlacement) abstract;
			virtual property Object^  Tag {Object^ get() { return tag; }; void set(Object^ value) { tag = value; }; }
			virtual property int Count {int get() abstract; }
			virtual IXbimGeometryObject^ Trim() abstract; 
			virtual void Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle) abstract;
		};

		ref class XbimGeometryObject abstract: IXbimGeometryObject 
		{
		private:
			Object^ tag;
		public:
			XbimGeometryObject(){};
#pragma region destructors

			virtual ~XbimGeometryObject() {};
			!XbimGeometryObject() {};

#pragma endregion
			virtual property  bool IsValid{bool  get() abstract; }
			virtual property bool IsSet{bool get() abstract; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() abstract;}
			virtual bool Equals(IXbimGeometryObject^ geom, double tolerance){ throw gcnew NotImplementedException("Function not implemented"); }
			virtual bool Intersects(IXbimGeometryObject^ geom, double tolerance){ throw gcnew NotImplementedException("Function not implemented"); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() abstract; };
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) abstract;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D) abstract;
			virtual property String^  ToBRep{String^ get(); }
			virtual property Object^  Tag {Object^ get() { return tag; }; void set(Object^ value) { tag = value; }; }
		};
	}
}

