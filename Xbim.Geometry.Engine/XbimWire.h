#pragma once
#include "XbimOccShape.h"
#include "XbimEdge.h"
#include "XbimVertex.h"
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <TopoDS_Edge.hxx>
#include <vector>
using namespace System;
using namespace System::Collections::Generic;
using namespace Xbim::Ifc4::Interfaces;
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
			void Init(IIfcPolyline^ loop);
			void Init(IIfcCompositeCurve^ loop);
			void Init(IIfcTrimmedCurve^ loop);
			void Init(IIfcCurve^ loop);
			void Init(IIfcIndexedPolyCurve^ pcurve);
			void Init(IIfcBSplineCurve^ bspline);
			void Init(IIfcBSplineCurveWithKnots^ bSpline);
			void Init(IIfcRationalBSplineCurveWithKnots^ bSpline);
			void Init(IIfcCompositeCurveSegment^ compCurveSeg);
			void Init(IIfcBoundedCurve^ loop);
			void Init(IIfcPolyLoop^ loop);
			void Init(IIfcArbitraryClosedProfileDef^ profile);
			void Init(IIfcArbitraryOpenProfileDef^ profile);
			void Init(IIfcCenterLineProfileDef^ profile);
			//parametrised profiles
			void Init(IIfcProfileDef^ profile);
			void Init(IIfcDerivedProfileDef^ profile);
			void Init(IIfcParameterizedProfileDef^ profile);
			void Init(IIfcCircleProfileDef^ circProfile);
			void Init(IIfcRectangleProfileDef^ rectProfile);
			void Init(IIfcRoundedRectangleProfileDef^ rectProfile);
			void Init(IIfcLShapeProfileDef^ profile);
			void Init(IIfcUShapeProfileDef^ profile);
			void Init(IIfcEllipseProfileDef^ profile);
			void Init(IIfcIShapeProfileDef^ profile);
			void Init(IIfcZShapeProfileDef^ profile);
			void Init(IIfcCShapeProfileDef^ profile);
			void Init(IIfcTShapeProfileDef^ profile);
			//constructs a rectangle wire with the bottom left corner at 0,0,0, top right at x,y,0
			void Init(double x, double y, double tolerance, bool centre);
#pragma endregion

			//helpers
			void AddNewellPoint(const gp_Pnt& previous, const gp_Pnt& current, double & x, double & y, double & z);
			bool AreEdgesC1(const TopoDS_Edge& e1, const TopoDS_Edge& e2, double precision, double angularTolerance);
		public:

#pragma region destructors

			~XbimWire() { InstanceCleanup(); }
			!XbimWire() { InstanceCleanup(); }
#pragma endregion

#pragma region constructors

			XbimWire() {}; //an empty invalid wire
			XbimWire(XbimEdge^ edge);

			XbimWire(double x, double y, double tolerance, bool centre);
			XbimWire(double precision);
			XbimWire(const std::vector<gp_Pnt>& points, double tolerance);
			XbimWire(const TopoDS_Wire& wire);
			XbimWire(const TopoDS_Wire& wire, Object^ tag);
			XbimWire(IIfcPolyline^ loop);
			XbimWire(IIfcBSplineCurve^ bspline);
			XbimWire(IIfcBSplineCurveWithKnots^ bSpline);
			XbimWire(IIfcRationalBSplineCurveWithKnots^ bSpline);
			XbimWire(IIfcCompositeCurve^ loop);
			XbimWire(IIfcTrimmedCurve^ loop);
			XbimWire(IIfcCurve^ loop);
			XbimWire(IIfcIndexedPolyCurve^ pcurve);
			XbimWire(IIfcCompositeCurveSegment^ compCurveSeg);
			XbimWire(IIfcBoundedCurve^ loop);
			XbimWire(IIfcPolyLoop^ loop);
			XbimWire(IIfcArbitraryClosedProfileDef^ profile);
			XbimWire(IIfcArbitraryOpenProfileDef^ profile);
			XbimWire(IIfcCenterLineProfileDef^ profile);
			//parametrised profiles
			XbimWire(IIfcProfileDef^ profile);
			XbimWire(IIfcDerivedProfileDef^ profile);
			XbimWire(IIfcParameterizedProfileDef^ profile);
			XbimWire(IIfcCircleProfileDef^ circProfile);
			XbimWire(IIfcRectangleProfileDef^ rectProfile);
			XbimWire(IIfcRoundedRectangleProfileDef^ rectProfile);
			XbimWire(IIfcLShapeProfileDef^ profile);
			XbimWire(IIfcUShapeProfileDef^ profile);
			XbimWire(IIfcEllipseProfileDef^ profile);
			XbimWire(IIfcIShapeProfileDef^ profile);
			XbimWire(IIfcZShapeProfileDef^ profile);
			XbimWire(IIfcCShapeProfileDef^ profile);
			XbimWire(IIfcTShapeProfileDef^ profile);


#pragma endregion


#pragma region operators
			operator const TopoDS_Wire& () { return *pWire; }
			virtual operator const TopoDS_Shape& () override { return *pWire; }
#pragma endregion


#pragma region IXbimWire Interface
			virtual property  XbimGeometryObjectType GeometryType {XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimWireType; }; }
			virtual property bool IsValid {bool get() override { return pWire != nullptr && !pWire->IsNull(); }; }
			virtual property IXbimEdgeSet^ Edges {IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices {IXbimVertexSet^ get(); }
			virtual property IEnumerable<XbimPoint3D>^ Points {IEnumerable<XbimPoint3D>^ get(); }
			//returns the normal of the loop using the Newell's normal algorithm
			virtual property XbimVector3D Normal {XbimVector3D get(); }
			virtual property bool IsClosed {bool get() { return IsValid && pWire->Closed() == Standard_True; }; }
			virtual property bool IsPlanar {bool get(); }
			virtual property XbimPoint3D Start {XbimPoint3D get(); }
			virtual property XbimPoint3D End {XbimPoint3D get(); }
			virtual IXbimWire^ Trim(double first, double last, double tolerance);

			virtual property double Length {double get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			void FuseColinearSegments(double tolerance, double angleTolerance);
			virtual property double Area {double get(); }
#pragma endregion

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimWire^ left, XbimWire^ right);
			static bool operator !=(XbimWire^ left, XbimWire^ right);
			virtual bool Equals(IXbimWire^ v);
#pragma endregion

			//properties
			property bool IsReversed {bool get() { return IsValid && pWire->Orientation() == TopAbs_REVERSED; }; }

			XbimWire^ Trim(XbimVertex^ first, XbimVertex^ last, double tolerance);



			//Returns the start parameter of each segment/interval of the wire
			virtual property List<double>^ IntervalParameters {List<double>^ get(); }
			//returns the point of each interval start, the last element is the end point of the last interval, comprises one more entry than IntervalParameters
			virtual property List<XbimPoint3D>^ IntervalPoints {List<XbimPoint3D>^ get(); }
			//functions 
			//Returns the point at the parameter
			XbimPoint3D PointAtParameter(double param);
			double ParameterAtPoint(XbimPoint3D point, double tolerance);
			//Methods

			//moves the face to the new position
			void Move(IIfcAxis2Placement3D^ position);
			//Translates the object by the translation vector
			void Translate(XbimVector3D translation);
			//change the direction of the loop
			void Reverse();
			XbimWire^  Reversed();
			array<ContourVertex>^ Contour();

			//Fillets all points with the specified radius
			bool FilletAll(double radius);


			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;


			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;

			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement) override;
			virtual void Move(TopLoc_Location loc);

			// Inherited via XbimOccShape
			virtual void Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle) override;
		};

		public ref class IfcPolylineComparer :IEqualityComparer<IIfcPolyline^>
		{
		public:
			// Inherited via IEqualityComparer
			virtual bool Equals(IIfcPolyline^ x, IIfcPolyline^ y);
			virtual int GetHashCode(IIfcPolyline^ obj);
			static bool IsSameDirection(IIfcPolyline^ pline, IXbimWire^ polyWire);
		};
	}

}