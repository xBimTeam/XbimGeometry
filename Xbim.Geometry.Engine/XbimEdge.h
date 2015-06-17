#pragma once
#include "XbimOccShape.h"
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>

using namespace System;
using namespace XbimGeometry::Interfaces;
using namespace Xbim::Ifc2x3::GeometryResource;
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
			void Init(IfcCurve^ edge);
			void Init(IfcConic^ edge);
			void Init(IfcCircle^ edge);
			void Init(IfcLine^ edge);
			void Init(IfcEllipse^ edge);
			void Init(IfcBSplineCurve^ bez);
			void Init(IfcBezierCurve^ bez);
			void Init(IfcRationalBezierCurve^ bez);

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
			XbimEdge(IfcCurve^ edge);
			XbimEdge(IfcConic^ edge);
			XbimEdge(IfcCircle^ edge);
			XbimEdge(IfcLine^ edge);
			XbimEdge(IfcEllipse^ edge);
			XbimEdge(IfcBSplineCurve^ bez);
			XbimEdge(IfcBezierCurve^ bez);
			XbimEdge(IfcRationalBezierCurve^ bez);
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
			virtual property double Length{double get(); }
			virtual property IXbimCurve^ EdgeGeometry{IXbimCurve^ get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
#pragma endregion	

#pragma region Properties
			property bool IsReversed{bool get(){ return IsValid && pEdge->Orientation() == TopAbs_REVERSED; }; }
#pragma endregion

		};
	}

}