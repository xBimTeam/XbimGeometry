#pragma once
#include "XbimSolid.h"
#include "XbimCompound.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
using namespace XbimGeometry::Interfaces;
using namespace System::Collections::Generic;
using namespace Xbim::Ifc2x3::TopologyResource;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimSolidSet : IXbimSolidSet
		{
		private:
			List<IXbimSolid^>^ solids;
			static XbimSolidSet^ empty = gcnew XbimSolidSet();
			void Init(IfcBooleanResult^ boolOp);
			void Init(XbimCompound^ comp, int label);
		public:
			static property XbimSolidSet^ Empty{XbimSolidSet^ get(){ return empty; }};
			XbimSolidSet();
			XbimSolidSet(const TopoDS_Shape& shape);
			XbimSolidSet(XbimCompound^ shape);
			XbimSolidSet(IXbimSolid^ solid);
			XbimSolidSet(IEnumerable<IXbimSolid^>^ solids);
			XbimSolidSet(IfcBooleanResult^ boolOp);
			XbimSolidSet(IfcManifoldSolidBrep^ solid);
			XbimSolidSet(IfcFacetedBrep^ solid);
			XbimSolidSet(IfcFacetedBrepWithVoids^ solid);
			XbimSolidSet(IfcClosedShell^ solid);


			virtual property bool IsValid{bool get(){ return Count>0; }; }
			virtual property bool IsSet{bool get()  { return true; }; }
			virtual property IXbimSolid^ First{IXbimSolid^ get(); }
			virtual property int Count{int get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get(); }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimSolidSetType; }}
			virtual IEnumerator<IXbimSolid^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); };
			virtual void Add(IXbimGeometryObject^ solid);
			virtual IXbimSolidSet^ Cut(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimSolidSet^ Cut(IXbimSolid^ solid, double tolerance);
			virtual IXbimSolidSet^ Union(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimSolidSet^ Union(IXbimSolid^ solid, double tolerance);
			virtual IXbimSolidSet^ Intersection(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimSolidSet^ Intersection(IXbimSolid^ solid, double tolerance);
			virtual property bool IsPolyhedron{ bool get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
		};

	}
}

