#pragma once
#include "XbimGeometryObject.h"
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shell.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Array1OfBox.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BOPAlgo_Operation.hxx>

using namespace System::Collections::Generic;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{


		ref class XbimGeometryObjectSet : XbimSetObject, IXbimGeometryObjectSet
		{

		private:
			List<IXbimGeometryObject^>^ geometryObjects;
			static XbimGeometryObjectSet^ empty = gcnew XbimGeometryObjectSet();
			
			static bool ParseGeometry(IEnumerable<IXbimGeometryObject^>^ geomObjects, TopTools_ListOfShape& toBeCut, Bnd_Array1OfBox& aBoxes,
				TopoDS_Shell& facesToIgnore, double tolerance);
			
			void InstanceCleanup()
			{
				geometryObjects = nullptr;
			};
			static TopoDS_Compound CreateCompound(IEnumerable<IXbimGeometryObject^>^ geomObjects);
		public:
			static property IXbimGeometryObjectSet^ Empty{IXbimGeometryObjectSet^ get(){ return empty; }};	
			static IXbimGeometryObjectSet^ PerformBoolean(BOPAlgo_Operation bop, IEnumerable<IXbimGeometryObject^>^ geomObjects, IXbimSolidSet^ solids, double tolerance);
			static IXbimGeometryObjectSet^ PerformBoolean(BOPAlgo_Operation bop, IXbimGeometryObject^ geomObject, IXbimSolidSet^ solids, double tolerance);

			XbimGeometryObjectSet::XbimGeometryObjectSet();
			XbimGeometryObjectSet(IEnumerable<IXbimGeometryObject^>^ objects);
			XbimGeometryObjectSet(int size){geometryObjects = gcnew List<IXbimGeometryObject^>(size);}

#pragma region destructors

			~XbimGeometryObjectSet(){ InstanceCleanup(); }
			!XbimGeometryObjectSet(){ InstanceCleanup(); }

#pragma endregion

#pragma region IXbimGeometryObjectSet Interface
			virtual property bool IsValid{bool get(){ return Count>0; }; }
			virtual property bool IsSet{bool get(){ return true; }; }
			virtual property IXbimGeometryObject^ First{IXbimGeometryObject^ get(); }
			virtual property int Count {int get() override; }
			virtual IXbimGeometryObject^ Trim()  override { if (Count == 1) return First; else if (Count == 0) return nullptr; else return this; };
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimGeometryObjectSetType; }}
			virtual IEnumerator<IXbimGeometryObject^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() ; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);
			virtual property IXbimSolidSet^ Solids{IXbimSolidSet^ get(); }
			virtual property IXbimShellSet^ Shells{IXbimShellSet^ get(); }
			virtual property IXbimFaceSet^ Faces{IXbimFaceSet^ get(); }
			virtual property IXbimEdgeSet^ Edges{IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices{IXbimVertexSet^ get(); }
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolid^ solid, double tolerance);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolid^ solid, double tolerance);
			virtual property String^  ToBRep{String^ get(); }
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolid^ solid, double tolerance);
			virtual bool Sew();
#pragma endregion
			virtual void Add(IXbimGeometryObject^ geomObj){ geometryObjects->Add(geomObj); }

			// Inherited via XbimSetObject
			virtual IXbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;
			virtual IXbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;
			virtual IXbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement) override;

			// Inherited via XbimSetObject
			virtual void Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle) override;
		};
	}
}
