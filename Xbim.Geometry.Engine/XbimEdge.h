#pragma once
#include "XbimOccShape.h"
#include "XbimCurve.h"
#include "XbimCurve2D.h"
#include "XbimVertex.h"
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include "XbimPoint3DWithTolerance.h"

using namespace System;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimEdge : IXbimEdge, XbimOccShape
		{
		private:
			
			IntPtr ptrContainer;
			virtual property TopoDS_Edge* pEdge
			{
				TopoDS_Edge* get() sealed { return (TopoDS_Edge*)ptrContainer.ToPointer(); }
				void set(TopoDS_Edge* val)sealed { ptrContainer = IntPtr(val); }
			}
			void InstanceCleanup();
#pragma region Initialisation
			void Init(IIfcCurve^ edge);
			void Init(IIfcConic^ edge);
			void Init(IIfcCircle^ edge);
			void Init(IIfcPolyline^ pline);
			void Init(IIfcLine^ edge);
			void Init(IIfcEllipse^ edge);
			void Init(IIfcBSplineCurve^ bSpline);
			void Init(IIfcBSplineCurveWithKnots^ bSpline);
			void Init(IIfcRationalBSplineCurveWithKnots^ bSpline);
			void Init(IIfcPcurve^ curve);
#pragma endregion

			XbimEdge(){};
		public:
			//error messages
			static String^ GetBuildEdgeErrorMessage(BRepBuilderAPI_EdgeError edgeErr);
			//Constructors and destructors
			~XbimEdge(){ InstanceCleanup(); }
			!XbimEdge(){ InstanceCleanup(); }

#pragma region Constructors
			XbimEdge(IXbimVertex^ edgeStart, IXbimVertex^ edgeEnd);
			XbimEdge(const TopoDS_Edge& edge);
			XbimEdge(const TopoDS_Edge& edge, Object^ tag);
			XbimEdge(IIfcCurve^ edge);
			XbimEdge(IIfcConic^ edge);
			XbimEdge(IIfcCircle^ edge);
			XbimEdge(IIfcLine^ edge);
			XbimEdge(IIfcEllipse^ edge);
			XbimEdge(IIfcBSplineCurve^ bSpline);
			XbimEdge(IIfcBSplineCurveWithKnots^ bSpline);
			XbimEdge(IIfcRationalBSplineCurveWithKnots^ bSpline);
			XbimEdge(IIfcPcurve^ pCurve);
			XbimEdge(XbimEdge^ edgeCurve, XbimVertex^ start, XbimVertex^ end, double maxTolerance);
			XbimEdge(const TopoDS_Wire& wire, double tolerance, double angleTolerance);
			XbimEdge(IIfcCurve^ edgeCurve, XbimVertex^ start, XbimVertex^ end);
			XbimEdge(XbimVertex^ start, XbimVertex^ midPoint, XbimVertex^ end);
			XbimEdge(XbimCurve^ curve3D);
			XbimEdge(XbimCurve2D^ curve2D);
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
			XbimEdge^ Reversed();

			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;

			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;
			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement) override;
			virtual void Move(TopLoc_Location loc);

			// Inherited via XbimOccShape
			virtual void Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle) override;
		};
	}

}