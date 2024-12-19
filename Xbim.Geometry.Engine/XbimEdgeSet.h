#pragma once

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include "XbimEdge.h"
#include "XbimWire.h"
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimEdgeSet : XbimSetObject, IXbimEdgeSet
		{
		private:			
			List<IXbimEdge^>^ edges;
			
			
			void InstanceCleanup()
			{
				edges = nullptr;
			};
		public:
			

#pragma region Constructors
			XbimEdgeSet(ModelGeometryService^ modelService) : XbimSetObject(modelService) { edges = gcnew List<IXbimEdge^>(); }
			XbimEdgeSet(const TopoDS_Shape& shape, ModelGeometryService^ modelService);
			IXCompound^ ToXCompound();
			operator  TopoDS_Shape () override;
			XbimEdgeSet(System::Collections::Generic::IEnumerable<IXbimEdge^>^ edges, ModelGeometryService^ modelService);
			XbimEdgeSet(XbimWire^ wire, ModelGeometryService^ modelService);
			
#pragma endregion

#pragma region destructors
			
			~XbimEdgeSet(){ InstanceCleanup(); }
			!XbimEdgeSet(){ InstanceCleanup(); }

#pragma endregion

#pragma region IXbimEdgeSet Interface
			virtual property bool IsValid{bool get(){ return Count>0; }; }
			virtual property bool IsSet{bool get() { return true; }; }
			virtual void Add(IXbimEdge^ edge) { edges->Add(edge); };
			virtual property IXbimEdge^ First{IXbimEdge^ get(); }
			virtual property int Count {int get() override; }
			virtual IXbimGeometryObject^ Trim()  override { if (Count == 1) return First; else if (Count == 0) return nullptr; else return this; };
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimEdgeSetType; }}
			virtual System::Collections::Generic::IEnumerator<IXbimEdge^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() ; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);
#pragma endregion

			// Inherited via XbimSetObject
			virtual IXbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;

			// Inherited via XbimSetObject
			virtual IXbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;
			virtual IXbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger) override;

			// Inherited via XbimSetObject
			virtual void Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle) override;
		};
		
	}
}

