#pragma once
#include "XbimSolid.h"
#include "XbimCompound.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
using namespace Xbim::Common;
using namespace System::Collections::Generic;

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


		ref class XbimSolidSet :XbimSetObject, IXbimSolidSet
		{
		private:
			List<IXbimSolid^>^ solids;
			static XbimSolidSet^ empty = gcnew XbimSolidSet();
			void Init(IIfcBooleanResult^ boolOp);
			void Init(XbimCompound^ comp, IPersistEntity^ ent);
			void Init(IIfcSweptAreaSolid^ solid);
			void Init(IIfcExtrudedAreaSolid^ solid);
			void Init(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid);
			void Init(IIfcRevolvedAreaSolid^ solid);
			

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
			XbimSolidSet(IIfcBooleanResult^ boolOp);
			XbimSolidSet(IIfcManifoldSolidBrep^ solid);
			XbimSolidSet(IIfcFacetedBrep^ solid);
			XbimSolidSet(IIfcFacetedBrepWithVoids^ solid);
			XbimSolidSet(IIfcClosedShell^ solid);

			XbimSolidSet(IIfcSweptAreaSolid^ solid);
			XbimSolidSet(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid);
			XbimSolidSet(IIfcExtrudedAreaSolid^ solid);
			XbimSolidSet(IIfcRevolvedAreaSolid^ solid);
			

			virtual property bool IsValid{bool get(){ return solids!=nullptr && Count>0; }; }
			virtual property bool IsSimplified{bool get(){ return _isSimplified; }; void set(bool val){ _isSimplified = val; } }
			virtual property bool IsSet{bool get()  { return true; }; }
			virtual property IXbimSolid^ First{IXbimSolid^ get(); }
			virtual property int Count {int get() override; }
			virtual IXbimGeometryObject^ Trim()  override { if (Count == 1) return First; else if (Count == 0) return nullptr; else return this; };
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
			virtual property String^  ToBRep{String^ get(); }
			virtual property bool IsPolyhedron{ bool get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);
			virtual property double Volume{double get(); }
			virtual IXbimSolidSet^ Range(int start, int count);
			//moves the solid set to the new position
			void Move(IIfcAxis2Placement3D^ position);
			

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

