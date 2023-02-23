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
		ref class XbimCompound : XbimOccShape, IXbimGeometryObjectSet
		{
		private:
			
			System::IntPtr ptrContainer;
			virtual property TopoDS_Compound* pCompound
			{
				TopoDS_Compound* get() sealed { return (TopoDS_Compound*)ptrContainer.ToPointer(); }
				void set(TopoDS_Compound* val)sealed { ptrContainer = System::IntPtr(val); }
			}
			XbimCompound(ModelGeometryService^ modelService) :XbimOccShape(modelService) {};
			
			bool _isSewn;
			double _sewingTolerance;
			void InstanceCleanup();
			bool WithinTolerance(const  TopoDS_Wire& topoOuterLoop, const TopoDS_Face& topoAdvancedFace, double _sewingTolerance);
			//Initialisers
			void Init(IIfcConnectedFaceSet^ faceSet,  ILogger^ logger);
			TopoDS_Shape InitFaces(System::Collections::Generic::IEnumerable<IIfcFace^>^ faces,IIfcRepresentationItem^ theItem, ILogger^ logger);
			TopoDS_Shape InitAdvancedFaces(System::Collections::Generic::IEnumerable<IIfcFace^>^ faces, ILogger^ logger);
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
			XbimFace^ BuildFace(List<System::Tuple<XbimWire^, IIfcPolyLoop^, bool>^>^ wires, IIfcFace^ face, ILogger^ logger);
			static void  GetConnected(HashSet<XbimSolid^>^ connected, Dictionary<XbimSolid^, HashSet<XbimSolid^>^>^ clusters, XbimSolid^ clusterAround);
			
			
		public:
			~XbimCompound(){ InstanceCleanup(); }
			!XbimCompound(){ InstanceCleanup(); }
			XbimCompound(double sewingTolerance, ModelGeometryService^ modelService);
			XbimCompound(const TopoDS_Shape& shape,ModelGeometryService^ modelService); 
			XbimCompound(const TopoDS_Compound& compound, bool sewn, double tolerance, ModelGeometryService^ modelService);
			XbimCompound(const TopoDS_Compound& compound, bool sewn, double tolerance, Object^ tag, ModelGeometryService^ modelService);
			XbimCompound(IIfcConnectedFaceSet^ faceSet, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcShellBasedSurfaceModel^ sbsm, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcFaceBasedSurfaceModel^ fbsm, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcManifoldSolidBrep^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcFacetedBrep^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcFacetedBrepWithVoids^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcAdvancedBrep^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcAdvancedBrepWithVoids^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcClosedShell^ solid, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcTriangulatedFaceSet^ faceSet, ILogger^ logger, ModelGeometryService^ modelService);
			XbimCompound(IIfcPolygonalFaceSet^ faceSet, ILogger^ logger, ModelGeometryService^ modelService);
			
#pragma region IXbimCompound Interface
			virtual property bool IsValid {bool get() override { return ptrContainer != System::IntPtr::Zero && Count > 0; }; }
			virtual property bool IsSet{bool get() override { return true; }; }
			virtual property  XbimGeometryObjectType GeometryType  {XbimGeometryObjectType  get()override { return XbimGeometryObjectType::XbimCompoundType; }}
			virtual property int Count{int get(); }
			virtual property IXbimGeometryObject^ First{IXbimGeometryObject^ get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D) override;
			static List<XbimSolid^>^  GetDiscrete(List<XbimSolid^>^%);
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
			virtual System::Collections::Generic::IEnumerator<IXbimGeometryObject^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{return GetEnumerator();}
			//Upgrades the result to the highest level and simplest object without loss of representation
			IXbimGeometryObject^ Upgrade();
			IXbimShell^ MakeShell();
			static XbimCompound^ Merge(IXbimSolidSet^ solids, double tolerance, ILogger^ logger, ModelGeometryService^ modelServices);
			XbimCompound^ Cut(XbimCompound^ solids, double tolerance, ILogger^ logger);
			XbimCompound^ Union(XbimCompound^ solids, double tolerance, ILogger^ logger);
			XbimCompound^ Intersection(XbimCompound^ solids, double tolerance, ILogger^ logger);
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