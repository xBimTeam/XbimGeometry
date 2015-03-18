#pragma once
#include "XbimOccShape.h"
#include "XbimVertex.h"
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <vector>
using namespace System;
using namespace System::Collections::Generic;
using namespace XbimGeometry::Interfaces;
using namespace Xbim::Ifc2x3::GeometryResource;
using namespace Xbim::Ifc2x3::ProfileResource;
using namespace Xbim::Ifc2x3::TopologyResource;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Tessellator;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimWire : IXbimWire, XbimOccShape
		{
		private:
			
			IntPtr ptrContainer;
			virtual property TopoDS_Wire* pWire
			{
				TopoDS_Wire* get() sealed { return (TopoDS_Wire*)ptrContainer.ToPointer(); }
				void set(TopoDS_Wire* val)sealed { ptrContainer = IntPtr(val); }
			}
			void InstanceCleanup();
#pragma region initialisation functions

			void Init(double precision);	
			void Init(IfcPolyline^ loop);
			void Init(IfcCompositeCurve^ loop);
			void Init(IfcTrimmedCurve^ loop);
			void Init(IfcCurve^ loop);
			void Init(IfcBSplineCurve^ bspline);
			void Init(IfcBezierCurve^ bez);
			void Init(IfcRationalBezierCurve^ bez);
			void Init(IfcCompositeCurveSegment^ compCurveSeg);
			void Init(IfcBoundedCurve^ loop);
			void Init(IfcPolyLoop ^ loop);
			void Init(IfcArbitraryClosedProfileDef^ profile);
			void Init(IfcArbitraryOpenProfileDef^ profile);
			void Init(IfcCenterLineProfileDef^ profile);
			//parametrised profiles
			void Init(IfcProfileDef ^ profile);
			void Init(IfcDerivedProfileDef ^ profile);
			void Init(IfcParameterizedProfileDef ^ profile);
			void Init(IfcCircleProfileDef ^ circProfile);
			void Init(IfcRectangleProfileDef^ rectProfile);
			void Init(IfcLShapeProfileDef ^ profile);
			void Init(IfcUShapeProfileDef ^ profile);
			void Init(IfcCraneRailFShapeProfileDef ^ profile);
			void Init(IfcCraneRailAShapeProfileDef ^ profile);
			void Init(IfcEllipseProfileDef ^ profile);
			void Init(IfcIShapeProfileDef ^ profile);
			void Init(IfcZShapeProfileDef ^ profile);
			void Init(IfcCShapeProfileDef ^ profile);
			void Init(IfcTShapeProfileDef ^ profile);
			//constructs a rectangle wire with the bottom left corner at 0,0,0, top right at x,y,0
			void Init(double x, double y, double tolerance);
#pragma endregion

			//helpers
			void AddNewellPoint(const gp_Pnt& previous, const gp_Pnt& current, double & x, double & y, double & z);
		public:
			
#pragma region destructors

			~XbimWire(){ InstanceCleanup(); }
			!XbimWire(){ InstanceCleanup(); }
#pragma endregion

#pragma region constructors

			XbimWire() {}; //an empty invalid wire
			XbimWire(double precision);
			XbimWire(const std::vector<gp_Pnt>& points, double tolerance);
			XbimWire(const TopoDS_Wire& wire);
			XbimWire(IfcPolyline^ loop);
			XbimWire(IfcBSplineCurve^ bspline);
			XbimWire(IfcBezierCurve^ bez);
			XbimWire(IfcRationalBezierCurve^ bez);
			XbimWire(IfcCompositeCurve^ loop);
			XbimWire(IfcTrimmedCurve^ loop);
			XbimWire(IfcCurve^ loop);
			XbimWire(IfcCompositeCurveSegment^ compCurveSeg);
			XbimWire(IfcBoundedCurve^ loop);
			XbimWire(IfcPolyLoop ^ loop);
			XbimWire(IfcArbitraryClosedProfileDef^ profile);
			XbimWire(IfcArbitraryOpenProfileDef^ profile);
			XbimWire(IfcCenterLineProfileDef^ profile);
			//parametrised profiles
			XbimWire(IfcProfileDef ^ profile);
			XbimWire(IfcDerivedProfileDef ^ profile);
			XbimWire(IfcParameterizedProfileDef ^ profile);
			XbimWire(IfcCircleProfileDef ^ circProfile);
			XbimWire(IfcRectangleProfileDef^ rectProfile);
			XbimWire(IfcLShapeProfileDef ^ profile);
			XbimWire(IfcUShapeProfileDef ^ profile);
			XbimWire(IfcCraneRailFShapeProfileDef ^ profile);
			XbimWire(IfcCraneRailAShapeProfileDef ^ profile);
			XbimWire(IfcEllipseProfileDef ^ profile);
			XbimWire(IfcIShapeProfileDef ^ profile);
			XbimWire(IfcZShapeProfileDef ^ profile);
			XbimWire(IfcCShapeProfileDef ^ profile);
			XbimWire(IfcTShapeProfileDef ^ profile);
			XbimWire(double x, double y, double tolerance);

#pragma endregion

			
#pragma region operators
			operator const TopoDS_Wire& () { return *pWire; }
			virtual operator const TopoDS_Shape& () override { return *pWire; }
#pragma endregion

			
#pragma region IXbimWire Interface
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override {return XbimGeometryObjectType::XbimWireType; }; }
			virtual property bool IsValid{bool get() override { return pWire != nullptr; }; }
			virtual property IXbimEdgeSet^ Edges{IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices{IXbimVertexSet^ get(); }
			virtual property IEnumerable<XbimPoint3D>^ Points{IEnumerable<XbimPoint3D>^ get(); }
			//returns the normal of the loop using the Newell's normal algorithm
			virtual property XbimVector3D Normal{XbimVector3D get(); }
			virtual property bool IsClosed{bool get(){ return IsValid && pWire->Closed() == Standard_True; }; }
			virtual property bool IsPlanar{bool get(); }	
			virtual property XbimPoint3D Start{XbimPoint3D get(); }
			virtual property XbimPoint3D End{XbimPoint3D get(); }
			virtual IXbimWire^ Trim(double first, double last, double tolerance);
			virtual property double Length{double get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;

#pragma endregion
	
#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimWire^ left, XbimWire^ right);
			static bool operator !=(XbimWire^ left, XbimWire^ right);
			virtual bool Equals(IXbimWire^ v);
#pragma endregion

			//properties
property bool IsReversed{bool get(){ return IsValid && pWire->Orientation() == TopAbs_REVERSED; }; }
			
			


			//Returns the start parameter of each segment/interval of the wire
			virtual property List<double>^ IntervalParameters{List<double>^ get(); }
			//returns the point of each interval start, the last element is the end point of the last interval, comprises one more entry than IntervalParameters
			virtual property List<XbimPoint3D>^ IntervalPoints{List<XbimPoint3D>^ get(); }
			//functions 
			//Returns the point at the parameter
			XbimPoint3D PointAtParameter(double param);

			//Methods

			//moves the face to the new position
			void Move(IfcAxis2Placement3D^ position);
			//Translates the object by the translation vector
			void Translate(XbimVector3D translation);
			//change the direction of the loop
			void Reverse();
			array<ContourVertex>^ Contour();
		
		};
	}
}