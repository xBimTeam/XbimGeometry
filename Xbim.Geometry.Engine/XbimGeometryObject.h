#pragma once
using namespace System;
using namespace Xbim::Common::Geometry;
using namespace System::Collections::Generic;
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

			~XbimGeometryObjectEnumerator()
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


		ref class XbimGeometryObject abstract: IXbimGeometryObject 
		{
		public:
			XbimGeometryObject();
#pragma region destructors

			~XbimGeometryObject() {};
			!XbimGeometryObject() {};

#pragma endregion
			virtual property  bool IsValid{bool  get() abstract; }
			virtual property bool IsSet{bool get() abstract; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() abstract;}
			virtual bool Equals(IXbimGeometryObject^ geom, double tolerance){ throw gcnew NotImplementedException("Function not implemented"); }
			virtual bool Intersects(IXbimGeometryObject^ geom, double tolerance){ throw gcnew NotImplementedException("Function not implemented"); }
			virtual property XbimRect3D BoundingBox{XbimRect3D get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) abstract;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D) abstract;
		};
	}
}

