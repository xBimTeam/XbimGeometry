#pragma once
#include <TopoDS_Shape.hxx>


using namespace Xbim::Common::Geometry;
using namespace System::Collections::Generic;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Ifc4;
using namespace Microsoft::Extensions::Logging;

using namespace Xbim::Geometry::Abstractions;


namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			ref class ModelGeometryService;
		}

		ref class XbimGeometryObjectEnumerator : System::Collections::Generic::IEnumerator<IXbimGeometryObject^>
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
		protected:
			Xbim::Geometry::Services::ModelGeometryService^ _modelServices;
		private:
			Object^ tag;
		public:
			XbimSetObject(Xbim::Geometry::Services::ModelGeometryService^ modelService)
			{
				_modelServices = modelService;
			}
			virtual operator TopoDS_Shape () abstract;
			virtual IXbimGeometryObject^ Transformed(IIfcCartesianTransformationOperator^ transformation) abstract;
			virtual IXbimGeometryObject^ Moved(IIfcPlacement^ placement) abstract;
			virtual IXbimGeometryObject^ Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger) abstract;
			virtual property Object^ Tag {Object^ get() { return tag; }; void set(Object^ value) { tag = value; }; }
			virtual property int Count {int get() abstract; }
			virtual IXbimGeometryObject^ Trim() abstract;
			virtual void Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle) abstract;
		};

		ref class XbimGeometryObject abstract : IXbimGeometryObject
		{
		protected:
			Xbim::Geometry::Services::ModelGeometryService^ _modelServices;
		private:
			Object^ tag;
			
		public:
			static IXShape^ ToXShape(IXbimGeometryObject^ geomObj);
			XbimGeometryObject(Xbim::Geometry::Services::ModelGeometryService^ modelServices) { _modelServices = modelServices;};
#pragma region destructors

			virtual ~XbimGeometryObject() {};
			!XbimGeometryObject() {};

#pragma endregion
			virtual property  bool IsValid {bool  get() abstract; }
			virtual property bool IsSet {bool get() abstract; }
			virtual property  XbimGeometryObjectType GeometryType {XbimGeometryObjectType  get() abstract; }
			virtual bool Equals(IXbimGeometryObject^, double) { throw gcnew System::NotImplementedException("Function not implemented"); }
			virtual bool Intersects(IXbimGeometryObject^, double) { throw gcnew System::NotImplementedException("Function not implemented"); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() abstract; };
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) abstract;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D) abstract;
			virtual property System::String^ ToBRep {System::String^ get(); }
			virtual property Object^ Tag {Object^ get() { return tag; }; void set(Object^ value) { tag = value; }; };

		};
	}
}

