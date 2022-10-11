#pragma once

#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include "XbimPoint3DWithTolerance.h"
#include "XbimOccShape.h"
#include "XbimCurve.h"
#include "XbimCurve2D.h"
#include "XbimVertex.h"

using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimEdgeV5 : IXbimEdge, XbimOccShape
		{
		private:
			
			System::IntPtr ptrContainer;
			virtual property TopoDS_Edge* pEdge
			{
				TopoDS_Edge* get() sealed { return (TopoDS_Edge*)ptrContainer.ToPointer(); }
				void set(TopoDS_Edge* val)sealed { ptrContainer = System::IntPtr(val); }
			}
			void InstanceCleanup();
#pragma region Initialisation
			void Init(IIfcCurve^ edge, ILogger^ logger);
			void Init(IIfcProfileDef^ profile, ILogger^ logger);
			void Init(IIfcArbitraryClosedProfileDef^ profile, ILogger^ logger);
			void Init(IIfcArbitraryOpenProfileDef^ profile, ILogger^ logger);
			void Init(IIfcParameterizedProfileDef^ profile, ILogger^ logger);
			void Init(IIfcDerivedProfileDef^ profile, ILogger^ logger);
			void Init(IIfcCenterLineProfileDef^ profile, ILogger^ logger);
			
#pragma endregion

			XbimEdgeV5(){};
		public:
			//error messages
			static System::String^ GetBuildEdgeErrorMessage(BRepBuilderAPI_EdgeError edgeErr);
			//Constructors and destructors
			~XbimEdgeV5(){ InstanceCleanup(); }
			!XbimEdgeV5(){ InstanceCleanup(); }

#pragma region Constructors
			XbimEdgeV5(IXbimVertex^ edgeStart, IXbimVertex^ edgeEnd);
			XbimEdgeV5(const TopoDS_Edge& edge);
			XbimEdgeV5(const TopoDS_Edge& edge, Object^ tag);
			XbimEdgeV5(IIfcCurve^ edge, ILogger^ logger);
			XbimEdgeV5(IIfcProfileDef^ profile, ILogger^ logger);
			XbimEdgeV5(XbimEdgeV5^ edgeCurve, XbimVertexV5^ start, XbimVertexV5^ end, double maxTolerance);
			XbimEdgeV5(const TopoDS_Wire& wire, double tolerance, double angleTolerance, ILogger^ logger);
			XbimEdgeV5(IIfcCurve^ edgeCurve, XbimVertexV5^ start, XbimVertexV5^ end, ILogger^ logger);
			XbimEdgeV5(XbimVertexV5^ start, XbimVertexV5^ midPoint, XbimVertexV5^ end);
			XbimEdgeV5(XbimCurveV5^ curve3D);
			XbimEdgeV5(Handle(Geom_Curve) curve3D);
			XbimEdgeV5(XbimCurve2DV5^ curve2D, ILogger^ logger);
			
#pragma endregion


			
#pragma region operators
			operator const TopoDS_Edge& () { return *pEdge; }
			virtual operator const TopoDS_Shape& () override { return *pEdge; }
#pragma endregion

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimEdgeV5^ left, XbimEdgeV5^ right);
			static bool operator !=(XbimEdgeV5^ left, XbimEdgeV5^ right);
			virtual bool Equals(IXbimEdge^ e);

#pragma endregion

#pragma region IXbim Edge Interfaces
			virtual property bool IsValid{bool get() override { return pEdge != nullptr; }; }

			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimEdgeType; }; }
			virtual property IXbimVertex^ EdgeStart{IXbimVertex^ get(); }
			virtual property IXbimVertex^ EdgeEnd{IXbimVertex^ get(); }
			virtual property XbimPoint3D EdgeStartPoint {XbimPoint3D get(); }
			virtual property XbimPoint3D EdgeEndPoint {XbimPoint3D get(); }
			virtual property double Length{double get(); }
			virtual property IXbimCurve^ EdgeGeometry{IXbimCurve^ get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual property bool IsClosed{bool get(){ return IsValid && pEdge->Closed() == Standard_True; }; }
#pragma endregion	

#pragma region Properties
			property bool IsReversed{bool get(){ return IsValid && pEdge->Orientation() == TopAbs_REVERSED; }; }
#pragma endregion
			void Reverse();
			XbimEdgeV5^ Reversed();

			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;

			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;
			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger) override;
			virtual void Move(TopLoc_Location loc);

			// Inherited via XbimOccShape
			virtual void Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle) override;
		};
	 
		ref class XbimBiPolarLinearEdge
		{
			XbimPoint3DWithTolerance^ pointA;
			XbimVertexV5^ vertexA;
			XbimPoint3DWithTolerance^ pointB;
			XbimVertexV5^ vertexB;
			XbimEdgeV5^ edgeAB;
			
			int refCount=0;
			int hashCode;
		public:
			XbimBiPolarLinearEdge(XbimPoint3DWithTolerance^ pA, XbimVertexV5^ vA, XbimPoint3DWithTolerance^ pB, XbimVertexV5^ vB)
			{
				pointA = pA;
				vertexA = vA;
				pointB = pB;
				vertexB = vB;
				int hashA = pointA->GetHashCode();
				int hashB = pointB->GetHashCode();
				hashCode = System::Math::Max(hashA, hashB) ^ System::Math::Min(hashA, hashB);
				if (hashA == hashB && pA == pB)
					refCount = -1;
				else
					edgeAB = gcnew XbimEdgeV5(vertexA, vertexB);
				
			}
			
			virtual property int ReferenceCount {int get() { return refCount; }}
			virtual property int IsEmptyLine {int get() { return refCount==-1; }}
			void ReleaseEdge() 
			{
				if (refCount > 0)refCount--;
			}
			XbimEdgeV5^ TakeEdge(XbimPoint3DWithTolerance^ pA)
			{
				if (IsEmptyLine) 
					return nullptr;
				refCount++;
				if (pA == pointA)
					return edgeAB;
				else
					return edgeAB->Reversed();
			    
			}

#pragma region Equality Overrides
			virtual bool Equals(Object^ obj) override
			{
				XbimBiPolarLinearEdge^ e = dynamic_cast<XbimBiPolarLinearEdge^>(obj);
				// Check for null
				if (e == nullptr) return false;
				return this == e;
			}
			
			virtual int GetHashCode() override
			{			
				return hashCode;
			}
			
			static bool operator ==(XbimBiPolarLinearEdge^ left, XbimBiPolarLinearEdge^ right)
			{
				// If both are null, or both are same instance, return true.
				if (System::Object::ReferenceEquals(left, right))
					return true;

				// If one is null, but not both, return false.
				if (((Object^)left == nullptr) || ((Object^)right == nullptr))
					return false;
				return (left->pointA == right->pointA && left->pointB == right->pointB) || (left->pointA == right->pointB && left->pointB == right->pointA);
			}
			static bool operator !=(XbimBiPolarLinearEdge^ left, XbimBiPolarLinearEdge^ right)
			{
				return !(left == right);
			}

#pragma endregion
		};
	}
}