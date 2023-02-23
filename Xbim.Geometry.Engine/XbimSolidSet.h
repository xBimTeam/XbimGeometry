#pragma once

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include "XbimSolid.h"
#include "XbimCompound.h"
#include "XbimGeometryObjectSet.h"

using namespace Xbim::Common;
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		const int BOOLEAN_PARTIALSUCCESSBADTOPOLOGY = 4; //we have managed to create a shape but it fails topo analysis
		const int BOOLEAN_PARTIALSUCCESSSINGLECUT = 3; //had to do each tool separately but one or more ops failed but we have something
		const int BOOLEAN_SUCCESSSINGLECUT = 2;//had to do each tool separately total sucess		
		const int BOOLEAN_SUCCESS = 1; //first attempt with all  tools worked
		const int BOOLEAN_FAIL = 0;
		const int BOOLEAN_TIMEDOUT = -1;


		

		private ref class VolumeComparer : System::Collections::Generic::IComparer<System::Tuple<double, XbimSolid^>^>
		{
		public:
			virtual int Compare(System::Tuple<double, XbimSolid^>^ x, System::Tuple<double, XbimSolid^>^ y)
			{
				// Compare y and x in reverse order. 
				return y->Item1.CompareTo(x->Item1);
			}
		};



		ref class XbimSolidSet :XbimSetObject, IXbimSolidSet
		{
		private:
			
			List<IXbimSolid^>^ solids;
			
			void Init(IIfcBooleanOperand^ boolOp, ILogger^ logger);
			void Init(IIfcBooleanResult^ boolOp, ILogger^ logger);
			void Init(IIfcBooleanClippingResult^ solid, ILogger^ logger);
			void Init(XbimCompound^ comp, IPersistEntity^ ent, ILogger^ logger);
			void Init(IIfcSweptAreaSolid^ solid, ILogger^ logger);
			void Init(IIfcExtrudedAreaSolid^ solid, ILogger^ logger);
			void Init(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^ logger);
			void Init(IIfcRevolvedAreaSolid^ solid, ILogger^ logger);
			void Init(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger);
			void Init(IIfcPolygonalFaceSet^ IIfcSolid, ILogger^ logger);
			void Init(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger);
			void Init(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger);
			void Init(IIfcCsgSolid^ IIfcSolid, ILogger^ logger);
			static VolumeComparer^ _volumeComparer = gcnew VolumeComparer();
			static int _maxOpeningsToCut = 100;
			static double _maxOpeningVolumePercentage = 0.0002;
			bool _isSimplified = false;
			int _ifcEntityLabel = 0;
			void InstanceCleanup()
			{
				solids = nullptr;
			};
			//IXbimSolidSet^ DoBoolean(IXbimSolidSet^ arguments, BOPAlgo_Operation operation, double tolerance, ILogger^ logger);

		public:

#pragma region destructors

			~XbimSolidSet() { InstanceCleanup(); }
			!XbimSolidSet() { InstanceCleanup(); }

#pragma endregion

			
			static XbimSolidSet^ BuildClippingList(IIfcBooleanClippingResult^ solid, List<IIfcBooleanOperand^>^ clipList, ILogger^ logger, ModelGeometryService^ modelService);
			static XbimSolidSet^ BuildBooleanResult(IIfcBooleanResult^ solid, IfcBooleanOperator operatorType, XbimSolidSet^ ops, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(ModelGeometryService^ modelService);
			IXCompound^ ToXCompound();
			XbimSolidSet(const TopoDS_Shape& shape, ModelGeometryService^ modelService);
			void InitSolidsFromShape(const TopoDS_Shape& shape);
			XbimSolidSet(XbimCompound^ shape, ModelGeometryService^ modelService);
			XbimSolidSet(IXbimSolid^ solid, ModelGeometryService^ modelService);
			XbimSolidSet(System::Collections::Generic::IEnumerable<IXbimSolid^>^ solids, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcBooleanResult^ boolOp, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcBooleanClippingResult^ solid,  ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcBooleanOperand^ boolOp, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcManifoldSolidBrep^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcFacetedBrep^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcFacetedBrepWithVoids^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcClosedShell^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcCsgSolid^ IIfcSolid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcSweptAreaSolid^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcExtrudedAreaSolid^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcRevolvedAreaSolid^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcPolygonalFaceSet^ IIfcSolid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimSolidSet(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			
			virtual property bool IsValid
			{
				bool get()
				{
					if (solids == nullptr) return false;
					for each (IXbimSolid ^ solid in solids)
					{
						if (solid->IsValid) return true; //we have at least one valid solid
					}
					return false;
				}
			}
			virtual property bool IsSimplified {bool get() { return _isSimplified; }; void set(bool val) { _isSimplified = val; } }
			virtual property int IfcEntityLabel {int get() { return _ifcEntityLabel; }; void set(int val) { _ifcEntityLabel = val; } }
			virtual property bool IsSet {bool get() { return true; }; }
			virtual property IXbimSolid^ First {IXbimSolid^ get(); }
			virtual property int Count {int get() override; }
			virtual IXbimGeometryObject^ Trim()  override { if (Count == 1) return First; else if (Count == 0) return nullptr; else return this; };
			virtual property XbimRect3D BoundingBox {XbimRect3D get(); }
			virtual property  XbimGeometryObjectType GeometryType {XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimSolidSetType; }}
			virtual System::Collections::Generic::IEnumerator<IXbimSolid^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); };
			virtual void Add(IXbimGeometryObject^ solid);
			virtual void Reverse();

			virtual IXbimSolidSet^ Cut(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);

			virtual IXbimSolidSet^ Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Union(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Union(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Intersection(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimSolidSet^ Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual property System::String^ ToBRep {System::String^ get(); }
			virtual property bool IsPolyhedron { bool get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D);
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);
			virtual property double Volume {double get(); }
			virtual IXbimSolidSet^ Range(int start, int count);
			//moves the solid set to the new position
			void Move(IIfcAxis2Placement3D^ position);


			// Inherited via XbimSetObject
			virtual IXbimGeometryObject^ Transformed(IIfcCartesianTransformationOperator^ transformation) override;


			// Inherited via XbimSetObject

			virtual IXbimGeometryObject^ Moved(IIfcPlacement^ placement) override;

			virtual IXbimGeometryObject^ Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger) override;


			// Inherited via XbimSetObject
			virtual void Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle) override;
			operator TopoDS_Shape () override;
		};

		ref class XbimSolidSetBoolOpParams
		{
		private:
			XbimSolid^ _body;
			XbimSolidSet^ _ops;
			double _tolerance;
			ILogger^ _logger;
			XbimSolidSet^ _result;
			bool _success = false;
			bool _useBody = false;
			BOPAlgo_Operation _operation;
		public:
			virtual property BOPAlgo_Operation Operation {BOPAlgo_Operation get() { return _operation; } void set(BOPAlgo_Operation val) { _operation = val; } }
			virtual property bool Success {bool get() { return _success; } void set(bool val) { _success = val; } }
			virtual property bool UseBody {bool get() { return _useBody; } void set(bool val) { _useBody = val; } }
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

