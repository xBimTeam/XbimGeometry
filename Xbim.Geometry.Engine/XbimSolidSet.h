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
			void Init(IIfcBooleanResult^ boolOp, ILogger^ logger);
			void Init(XbimCompound^ comp, IPersistEntity^ ent, ILogger^ logger);
			void Init(IIfcSweptAreaSolid^ solid, ILogger^ logger);
			void Init(IIfcExtrudedAreaSolid^ solid, ILogger^ logger);
			void Init(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^ logger);
			void Init(IIfcRevolvedAreaSolid^ solid, ILogger^ logger);
			void Init(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger);
			void Init(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger);
			void Init(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger);

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
			XbimSolidSet(IIfcBooleanResult^ boolOp, ILogger^ logger);
			XbimSolidSet(IIfcManifoldSolidBrep^ solid, ILogger^ logger);
			XbimSolidSet(IIfcFacetedBrep^ solid, ILogger^ logger);
			XbimSolidSet(IIfcFacetedBrepWithVoids^ solid, ILogger^ logger);
			XbimSolidSet(IIfcClosedShell^ solid, ILogger^ logger);

			XbimSolidSet(IIfcSweptAreaSolid^ solid, ILogger^ logger);
			XbimSolidSet(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^ logger);
			XbimSolidSet(IIfcExtrudedAreaSolid^ solid, ILogger^ logger);
			XbimSolidSet(IIfcRevolvedAreaSolid^ solid, ILogger^ logger);
			XbimSolidSet(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger);
			XbimSolidSet(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger);
			XbimSolidSet(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger);

			virtual property bool IsValid {bool get() { return solids != nullptr && this != XbimSolidSet::Empty; }; }
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
			virtual IXbimSolidSet^ Cut(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Union(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Union(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Intersection(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger);
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

			virtual IXbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger) override;


			// Inherited via XbimSetObject
			virtual void Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle) override;

		};

		ref class XbimSolidSetBoolOpParams
		{
		private:
			XbimSolid^ _body;
			XbimSolidSet^ _ops;
			double _tolerance;
			ILogger^ _logger;
			XbimSolidSet^ _result;
		public:
			virtual property XbimSolid^ Body {XbimSolid^ get() { return _body; }}
			virtual property XbimSolidSet^ Ops {XbimSolidSet^ get() { return _ops; }}
			virtual property double Tolerance {double get() { return _tolerance; }}
			virtual property ILogger^ Logger {ILogger^ get() { return _logger; }}
			virtual property XbimSolidSet^ Result {XbimSolidSet^ get() { return _result; } void set(XbimSolidSet^ value) { _result = value; }}
			XbimSolidSetBoolOpParams(XbimSolid^ body, XbimSolidSet^ ops, double tolerance, ILogger^ logger)
			{
				_body = body;
				_ops = ops;
				_tolerance = tolerance;
				_logger = logger;
			}
		};

	}
}

