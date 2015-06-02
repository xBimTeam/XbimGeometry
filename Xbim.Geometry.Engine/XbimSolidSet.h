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
		private ref class VolumeComparer : IComparer<Tuple<double, XbimSolid^>^>
		{
		public:
			virtual int Compare(Tuple<double, XbimSolid^>^ x, Tuple<double, XbimSolid^>^ y)
			{
				// Compare y and x in reverse order. 
				return y->Item1.CompareTo(x->Item1);
			}
		};


		ref class XbimSolidSet : IXbimSolidSet
		{
		private:
			List<IXbimSolid^>^ solids;
			static XbimSolidSet^ empty = gcnew XbimSolidSet();
			void Init(IfcBooleanResult^ boolOp);
			void Init(XbimCompound^ comp, int label);
			static VolumeComparer^ _volumeComparer = gcnew VolumeComparer();
			static int _maxOpeningsToCut = 100;
			static double _maxOpeningVolumePercentage = 0.0002;
			bool _isSimplified = false;
			void InstanceCleanup()
			{
				solids = nullptr;
			};
		public:

#pragma region destructors

			~XbimSolidSet(){ InstanceCleanup(); }
			!XbimSolidSet(){ InstanceCleanup(); }

#pragma endregion

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
			virtual property bool IsSimplified{bool get(){ return _isSimplified; }; void set(bool val){ _isSimplified = val; } }
			virtual property bool IsSet{bool get()  { return true; }; }
			virtual property IXbimSolid^ First{IXbimSolid^ get(); }
			virtual property int Count{int get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get(); }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimSolidSetType; }}
			virtual IEnumerator<IXbimSolid^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); };
			virtual void Add(IXbimGeometryObject^ solid);
			virtual void Reverse();
			virtual IXbimSolidSet^ Cut(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimSolidSet^ Cut(IXbimSolid^ solid, double tolerance);
			virtual IXbimSolidSet^ Union(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimSolidSet^ Union(IXbimSolid^ solid, double tolerance);
			virtual IXbimSolidSet^ Intersection(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimSolidSet^ Intersection(IXbimSolid^ solid, double tolerance);
			virtual property bool IsPolyhedron{ bool get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
			virtual property double Volume{double get(); }
			
		};

	}
}

