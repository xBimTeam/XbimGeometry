#pragma once


#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp_Explorer.hxx>
#include <Precision.hxx>

#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimFace.h"


using namespace Microsoft::Extensions::Logging;
using namespace Xbim::Ifc4;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		//a set of Xbim Geometry Objects
		ref class XbimCompoundV5 : XbimOccShape, IXbimGeometryObjectSet
		{
		private:
			
			System::IntPtr ptrContainer;
			virtual property TopoDS_Compound* pCompound
			{
				TopoDS_Compound* get() sealed { return (TopoDS_Compound*)ptrContainer.ToPointer(); }
				void set(TopoDS_Compound* val)sealed { ptrContainer = System::IntPtr(val); }
			}
			XbimCompoundV5(){};
			static XbimCompoundV5^ empty = gcnew XbimCompoundV5();
			bool _isSewn;
			double _sewingTolerance;
			void InstanceCleanup();
			bool WithinTolerance(const  TopoDS_Wire& topoOuterLoop, const TopoDS_Face& topoAdvancedFace, double _sewingTolerance);
			//Initialisers
			void Init(IIfcConnectedFaceSet^ faceSet,  ILogger^ logger);
			TopoDS_Shape InitFaces(IEnumerable<IIfcFace^>^ faces,IIfcRepresentationItem^ theItem, ILogger^ logger);
			TopoDS_Shape InitAdvancedFaces(IEnumerable<IIfcFace^>^ faces, ILogger^ logger);
			void Init(IIfcShellBasedSurfaceModel^ sbsm, ILogger^ logger);
			void Init(IIfcFaceBasedSurfaceModel^ fbsm, ILogger^ logger);
			void Init(IIfcManifoldSolidBrep^ solid, ILogger^ logger);
			void Init(IIfcFacetedBrep^ solid, ILogger^ logger);
			void Init(IIfcFacetedBrepWithVoids^ solid, ILogger^ logger);
			void Init(IIfcAdvancedBrep^ solid, ILogger^ logger);
			void Init(IIfcAdvancedBrepWithVoids^ solid, ILogger^ logger);
			void Init(IIfcClosedShell^ solid, ILogger^ logger);
			void Init(IIfcOpenShell^ solid, ILogger^ logger);
			void Init(IIfcTriangulatedFaceSet^ faceSet, ILogger^ logger);
			
			//Helpers
			XbimFaceV5^ BuildFace(List<System::Tuple<XbimWireV5^, IIfcPolyLoop^, bool>^>^ wires, IIfcFace^ face, ILogger^ logger);
			static void  GetConnected(HashSet<XbimSolidV5^>^ connected, Dictionary<XbimSolidV5^, HashSet<XbimSolidV5^>^>^ clusters, XbimSolidV5^ clusterAround);
			
			
		public:
			~XbimCompoundV5(){ InstanceCleanup(); }
			!XbimCompoundV5(){ InstanceCleanup(); }
			XbimCompoundV5(double sewingTolerance);
			XbimCompoundV5(const TopoDS_Compound& compound, bool sewn, double tolerance);
			XbimCompoundV5(const TopoDS_Compound& compound, bool sewn, double tolerance, Object^ tag);
			XbimCompoundV5(IIfcConnectedFaceSet^ faceSet, ILogger^ logger);
			XbimCompoundV5(IIfcShellBasedSurfaceModel^ sbsm, ILogger^ logger);
			XbimCompoundV5(IIfcFaceBasedSurfaceModel^ fbsm, ILogger^ logger);
			XbimCompoundV5(IIfcManifoldSolidBrep^ solid, ILogger^ logger);
			XbimCompoundV5(IIfcFacetedBrep^ solid, ILogger^ logger);
			XbimCompoundV5(IIfcFacetedBrepWithVoids^ solid, ILogger^ logger);
			XbimCompoundV5(IIfcAdvancedBrep^ solid, ILogger^ logger);
			XbimCompoundV5(IIfcAdvancedBrepWithVoids^ solid, ILogger^ logger);
			XbimCompoundV5(IIfcClosedShell^ solid, ILogger^ logger);
			XbimCompoundV5(IIfcTriangulatedFaceSet^ faceSet, ILogger^ logger);
			XbimCompoundV5(IIfcPolygonalFaceSet^ faceSet, ILogger^ logger);
			static property XbimCompoundV5^ Empty{XbimCompoundV5^ get(){ return empty; }};
#pragma region IXbimCompound Interface
			virtual property bool IsValid {bool get() override { return ptrContainer != System::IntPtr::Zero && Count > 0; }; }
			virtual property bool IsSet{bool get() override { return true; }; }
			virtual property  XbimGeometryObjectType GeometryType  {XbimGeometryObjectType  get()override { return XbimGeometryObjectType::XbimCompoundType; }}
			virtual property int Count{int get(); }
			virtual property IXbimGeometryObject^ First{IXbimGeometryObject^ get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D) override;
			static List<XbimSolidV5^>^  GetDiscrete(List<XbimSolidV5^>^%);
			virtual property IXbimSolidSet^ Solids {IXbimSolidSet^ get(); }
			virtual property IXbimShellSet^ Shells{IXbimShellSet^ get(); }
			virtual property IXbimFaceSet^ Faces{IXbimFaceSet^ get(); }
			virtual property IXbimEdgeSet^ Edges{IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices{IXbimVertexSet^ get(); }
			virtual void Add(IXbimGeometryObject^ geomObj);
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual bool Sew();
			virtual bool Sew(ILogger^ logger);
#ifdef OCC_6_9_SUPPORTED //OCC 6.9.0. is better with complex booleans
			static int MaxFacesToSew = 3000;
#else
			static int MaxFacesToSew = 1000;
#endif
#pragma endregion
			//operators
			operator const TopoDS_Compound& () { return *pCompound; }
			virtual operator const TopoDS_Shape& () override { return *pCompound; }
			virtual property bool IsSewn{bool get(){ return _isSewn; }}
			virtual IEnumerator<IXbimGeometryObject^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{return GetEnumerator();}
			//Upgrades the result to the highest level and simplest object without loss of representation
			IXbimGeometryObject^ Upgrade();
			IXbimShell^ MakeShell();
			static XbimCompoundV5^ Merge(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			XbimCompoundV5^ Cut(XbimCompoundV5^ solids, double tolerance, ILogger^ logger);
			XbimCompoundV5^ Union(XbimCompoundV5^ solids, double tolerance, ILogger^ logger);
			XbimCompoundV5^ Intersection(XbimCompoundV5^ solids, double tolerance, ILogger^ logger);
			virtual property XbimRect3D BoundingBox {XbimRect3D get()override ; }
			virtual property double Volume{double get(); }
			virtual property double SewingTolerance {double get() {return _sewingTolerance;}}
			
			//moves the compound to the new positio
			void Move(IIfcAxis2Placement3D^ position);

			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;

			// Inherited via XbimOccShape
			virtual void Move(TopLoc_Location loc);
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;
			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger) override;
};

	}
}