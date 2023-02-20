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
		ref class XbimEdge : IXbimEdge, XbimOccShape
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
		public:
			XbimEdge(ModelGeometryService^ modelService) : XbimOccShape(modelService) {};

			//error messages
			static System::String^ GetBuildEdgeErrorMessage(BRepBuilderAPI_EdgeError edgeErr);
			//Constructors and destructors
			~XbimEdge() { InstanceCleanup(); }
			!XbimEdge() { InstanceCleanup(); }

#pragma region Constructors
			XbimEdge(IXbimVertex^ edgeStart, IXbimVertex^ edgeEnd, ModelGeometryService^ modelService);
			XbimEdge(const TopoDS_Edge& edge, ModelGeometryService^ modelService);
			XbimEdge(const TopoDS_Edge& edge, Object^ tag, ModelGeometryService^ modelService);
			XbimEdge(IIfcCurve^ edge, ILogger^ logger, ModelGeometryService^ modelService);
			XbimEdge(IIfcProfileDef^ profile, ILogger^ logger, ModelGeometryService^ modelService);
			XbimEdge(XbimEdge^ edgeCurve, XbimVertex^ start, XbimVertex^ end, double maxTolerance, ModelGeometryService^ modelService);
			XbimEdge(const TopoDS_Wire& wire, double tolerance, double angleTolerance, ILogger^ logger, ModelGeometryService^ modelService);
			XbimEdge(IIfcCurve^ edgeCurve, XbimVertex^ start, XbimVertex^ end, ILogger^ logger, ModelGeometryService^ modelService);
			XbimEdge(XbimVertex^ start, XbimVertex^ midPoint, XbimVertex^ end, ModelGeometryService^ modelService);
			XbimEdge(XbimCurve^ curve3D, ModelGeometryService^ modelService);
			XbimEdge(Handle(Geom_Curve) curve3D, ModelGeometryService^ modelService);
			XbimEdge(XbimCurve2D^ curve2D, ILogger^ logger, ModelGeometryService^ modelService);

#pragma endregion



#pragma region operators
			operator const TopoDS_Edge& () { return *pEdge; }
			virtual operator const TopoDS_Shape& () override { return *pEdge; }
#pragma endregion

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimEdge^ left, XbimEdge^ right);
			static bool operator !=(XbimEdge^ left, XbimEdge^ right);
			virtual bool Equals(IXbimEdge^ e);

#pragma endregion

#pragma region IXbim Edge Interfaces
			virtual property bool IsValid {bool get() override { return pEdge != nullptr; }; }

			virtual property  XbimGeometryObjectType GeometryType {XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimEdgeType; }; }
			virtual property IXbimVertex^ EdgeStart {IXbimVertex^ get(); }
			virtual property IXbimVertex^ EdgeEnd {IXbimVertex^ get(); }
			virtual property XbimPoint3D EdgeStartPoint {XbimPoint3D get(); }
			virtual property XbimPoint3D EdgeEndPoint {XbimPoint3D get(); }
			virtual property double Length {double get(); }
			virtual property IXbimCurve^ EdgeGeometry {IXbimCurve^ get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual property bool IsClosed {bool get() { return IsValid && pEdge->Closed() == Standard_True; }; }
#pragma endregion	

#pragma region Properties
			property bool IsReversed {bool get() { return IsValid && pEdge->Orientation() == TopAbs_REVERSED; }; }
#pragma endregion
			void Reverse();
			XbimEdge^ Reversed();

			// Inherited via XbimOccShape
			virtual XbimGeometryObject^ Transformed(IIfcCartesianTransformationOperator^ transformation) override;

			// Inherited via XbimOccShape
			virtual XbimGeometryObject^ Moved(IIfcPlacement^ placement) override;
			virtual XbimGeometryObject^ Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger) override;
			virtual void Move(TopLoc_Location loc);

			// Inherited via XbimOccShape
			virtual void Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle) override;
		};

		ref class XbimBiPolarLinearEdge
		{
			XbimPoint3DWithTolerance^ pointA;
			XbimVertex^ vertexA;
			XbimPoint3DWithTolerance^ pointB;
			XbimVertex^ vertexB;
			XbimEdge^ edgeAB;
			ModelGeometryService^ _modelService;
			int refCount = 0;
			int hashCode;
		public:
			XbimBiPolarLinearEdge(XbimPoint3DWithTolerance^ pA, XbimVertex^ vA, XbimPoint3DWithTolerance^ pB, XbimVertex^ vB, ModelGeometryService^ modelService)
			{
				_modelService = modelService;
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
					edgeAB = gcnew XbimEdge(vertexA, vertexB, _modelService);

			}

			virtual property int ReferenceCount {int get() { return refCount; }}
			virtual property int IsEmptyLine {int get() { return refCount == -1; }}
			void ReleaseEdge()
			{
				if (refCount > 0)refCount--;
			}
			XbimEdge^ TakeEdge(XbimPoint3DWithTolerance^ pA)
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