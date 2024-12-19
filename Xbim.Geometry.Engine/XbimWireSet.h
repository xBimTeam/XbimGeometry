#pragma once

#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>

#include "XbimWire.h"
using namespace System::Collections::Generic;
using namespace System::Collections;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimWireSet : XbimSetObject, IXbimWireSet
		{
		private:
			List<IXbimWire^>^ wires;

			
			void InstanceCleanup()
			{
				wires = nullptr;
			};
		public:

#pragma region Constructors
			IXCompound^ ToXCompound();
			XbimWireSet(ModelGeometryService^ modelService) :XbimSetObject(modelService) { wires = gcnew List<IXbimWire^>(1); }
			XbimWireSet(const TopoDS_Shape& shape, ModelGeometryService^ modelService);
			XbimWireSet(const TopTools_ListOfShape& wires, ModelGeometryService^ modelService);
			XbimWireSet(System::Collections::Generic::IEnumerable<IXbimWire^>^ wires, ModelGeometryService^ modelService) :XbimSetObject(modelService) { this->wires = gcnew List<IXbimWire^>(wires); };
#pragma endregion

#pragma region destructors

			~XbimWireSet() { InstanceCleanup(); }
			!XbimWireSet() { InstanceCleanup(); }

#pragma endregion

#pragma region operators
			virtual operator TopoDS_Shape () override;
			virtual property IXbimWire^ default[int]
			{
				IXbimWire^ get(int index)
				{
					return wires[index];
				}

				void set(int index, IXbimWire ^ value)
				{
					wires[index] = value;
				}
			}
				property XbimWire^ Wire[int]
			{
				XbimWire ^ get(int index)
				{
					return (XbimWire^)wires[index];
				}

				void set(int index, XbimWire ^ value)
				{
					wires[index] = value;
				}
			}
#pragma endregion

			virtual property bool IsValid {bool get() { return Count > 0; }; }
			virtual property bool IsSet {bool get() { return true; }; }
			virtual void Add(IXbimWire^ wire) { wires->Add(wire); };
			virtual property IXbimWire^ First {IXbimWire^ get(); }
			virtual property int Count {int get() override; }
			virtual IXbimGeometryObject^ Trim()  override { if (Count == 1) return First; else if (Count == 0) return nullptr; else return this; };
			virtual property XbimRect3D BoundingBox {XbimRect3D get(); }
			virtual property  XbimGeometryObjectType GeometryType {XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimWireSetType; }}
			virtual System::Collections::Generic::IEnumerator<IXbimWire^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D);
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);

			// Inherited via XbimSetObject
			virtual IXbimGeometryObject^ Transformed(IIfcCartesianTransformationOperator^ transformation) override;
			virtual IXbimGeometryObject^ Moved(IIfcPlacement^ placement) override;
			virtual IXbimGeometryObject^ Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger) override;

			// Inherited via XbimSetObject
			virtual void Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle) override;
		};

	}
}

