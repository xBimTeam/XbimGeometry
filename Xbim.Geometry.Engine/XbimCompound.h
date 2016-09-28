#pragma once
#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimFace.h"

#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp_Explorer.hxx>
#include <Precision.hxx>

using namespace System::Collections::Generic;
using namespace Xbim::Ifc4;
using namespace Xbim::Common::Geometry;

namespace Xbim
{
	namespace Geometry
	{
		//a set of Xbim Geometry Objects
		ref class XbimCompound : XbimOccShape, IXbimGeometryObjectSet
		{
		private:
			
			IntPtr ptrContainer;
			virtual property TopoDS_Compound* pCompound
			{
				TopoDS_Compound* get() sealed { return (TopoDS_Compound*)ptrContainer.ToPointer(); }
				void set(TopoDS_Compound* val)sealed { ptrContainer = IntPtr(val); }
			}
			XbimCompound(){};
			static XbimCompound^ empty = gcnew XbimCompound();
			bool _isSewn;
			double _sewingTolerance;
			void InstanceCleanup();
			//Initialisers
			void Init(IIfcConnectedFaceSet^ faceSet);
			void Init(IEnumerable<IIfcFace^>^ faces);
			XbimShell^ InitAdvancedFaces(IEnumerable<IIfcFace^>^ faces);
			void Init(IIfcShellBasedSurfaceModel^ sbsm);
			void Init(IIfcFaceBasedSurfaceModel^ fbsm);
			void Init(IIfcManifoldSolidBrep^ solid);
			void Init(IIfcFacetedBrep^ solid);
			void Init(IIfcFacetedBrepWithVoids^ solid);
			void Init(IIfcAdvancedBrep^ solid);
			void Init(IIfcAdvancedBrepWithVoids^ solid);
			void Init(IIfcClosedShell^ solid);
			void Init(IIfcTriangulatedFaceSet^ faceSet);
			//Helpers
			XbimFace^ BuildFace(List<Tuple<XbimWire^, IIfcPolyLoop^, bool>^>^ wires, IIfcFace^ face);
			static void  GetConnected(HashSet<XbimSolid^>^ connected, Dictionary<XbimSolid^, HashSet<XbimSolid^>^>^ clusters, XbimSolid^ clusterAround);
			
			
		public:
			~XbimCompound(){ InstanceCleanup(); }
			!XbimCompound(){ InstanceCleanup(); }
			XbimCompound(double sewingTolerance);
			XbimCompound(const TopoDS_Compound& compound, bool sewn, double tolerance);
			XbimCompound(const TopoDS_Compound& compound, bool sewn, double tolerance, Object^ tag);
			XbimCompound(IIfcConnectedFaceSet^ faceSet);
			XbimCompound(IIfcShellBasedSurfaceModel^ sbsm);
			XbimCompound(IIfcFaceBasedSurfaceModel^ fbsm);
			XbimCompound(IIfcManifoldSolidBrep^ solid);
			XbimCompound(IIfcFacetedBrep^ solid);
			XbimCompound(IIfcFacetedBrepWithVoids^ solid);
			XbimCompound(IIfcAdvancedBrep^ solid);
			XbimCompound(IIfcAdvancedBrepWithVoids^ solid);
			XbimCompound(IIfcClosedShell^ solid);
			XbimCompound(IIfcTriangulatedFaceSet^ faceSet);
			static property XbimCompound^ Empty{XbimCompound^ get(){ return empty; }};
#pragma region IXbimCompound Interface
			virtual property bool IsValid {bool get()override{ return ptrContainer != IntPtr::Zero && Count > 0; }; }
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
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolid^ solid, double tolerance);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolid^ solid, double tolerance);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolid^ solid, double tolerance);
			virtual bool Sew();
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
			static XbimCompound^ Merge(IXbimSolidSet^ solids, double tolerance);
			XbimCompound^ Cut(XbimCompound^ solids, double tolerance);
			XbimCompound^ Union(XbimCompound^ solids, double tolerance);
			XbimCompound^ Intersection(XbimCompound^ solids, double tolerance);
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
			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement) override;
};

	}
}