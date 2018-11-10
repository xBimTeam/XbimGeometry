#pragma once
#include "XbimOccShape.h"
#include "XbimEdge.h"
#include "XbimVertex.h"
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <TopoDS_Edge.hxx>
#include <vector>
#include <NCollection_Vector.hxx>
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

			// Lock for preventing a usage of BRepOffsetAPI_MakeOffset.Perform(...) in a multi-threaded mode.
			static Object^ _makeOffsetLock = gcnew Object();

			IntPtr ptrContainer;
			virtual property TopoDS_Wire* pWire
			{
				TopoDS_Wire* get() sealed { return (TopoDS_Wire*)ptrContainer.ToPointer(); }
				void set(TopoDS_Wire* val)sealed { ptrContainer = IntPtr(val); }
			}
			void InstanceCleanup();
#pragma region initialisation functions

			void Init(double precision);
			void Init(IIfcPolyline^ loop, ILogger^ logger);
			void Init(IIfcPolyline^ loop, bool attemptClosing, ILogger^ logger);
			void Init(IIfcCompositeCurve^ loop, ILogger^ logger);
			void Init(IIfcTrimmedCurve^ loop, ILogger^ logger);
			void Init(IIfcCurve^ loop, ILogger^ logger);
			void Init(IIfcIndexedPolyCurve^ pcurve, ILogger^ logger);
			void Init(IIfcBSplineCurve^ bspline, ILogger^ logger);
			void Init(IIfcBSplineCurveWithKnots^ bSpline, ILogger^ logger);
			void Init(IIfcRationalBSplineCurveWithKnots^ bSpline, ILogger^ logger);
			void Init(IIfcCompositeCurveSegment^ compCurveSeg, ILogger^ logger);
			void Init(IIfcBoundedCurve^ loop, ILogger^ logger);
			void Init(IIfcPolyLoop^ loop, ILogger^ logger);
			void Init(IIfcArbitraryClosedProfileDef^ profile, ILogger^ logger);
			void Init(IIfcArbitraryOpenProfileDef^ profile, ILogger^ logger);
			void Init(IIfcCenterLineProfileDef^ profile, ILogger^ logger);
			//parametrised profiles
			void Init(IIfcProfileDef^ profile, ILogger^ logger);
			void Init(IIfcDerivedProfileDef^ profile, ILogger^ logger);
			void Init(IIfcParameterizedProfileDef^ profile, ILogger^ logger);
			void Init(IIfcCircleProfileDef^ circProfile, ILogger^ logger);
			void Init(IIfcRectangleProfileDef^ rectProfile, ILogger^ logger);
			void Init(IIfcRoundedRectangleProfileDef^ rectProfile, ILogger^ logger);
			void Init(IIfcLShapeProfileDef^ profile, ILogger^ logger);
			void Init(IIfcUShapeProfileDef^ profile, ILogger^ logger);
			void Init(IIfcEllipseProfileDef^ profile, ILogger^ logger);
			void Init(IIfcIShapeProfileDef^ profile, ILogger^ logger);
			void Init(IIfcZShapeProfileDef^ profile, ILogger^ logger);
			void Init(IIfcCShapeProfileDef^ profile, ILogger^ logger);
			void Init(IIfcTShapeProfileDef^ profile, ILogger^ logger);
			//constructs a rectangle wire with the bottom left corner at 0,0,0, top right at x,y,0
			void Init(double x, double y, double tolerance, bool centre);
#pragma endregion

			//helpers
			void AddNewellPoint(const gp_Pnt& previous, const gp_Pnt& current, double & x, double & y, double & z);
			bool AreEdgesC1(const TopoDS_Edge& e1, const TopoDS_Edge& e2, double precision, double angularTolerance);
			bool SortEdgesForWire(const NCollection_Vector<TopoDS_Edge>& oldedges, NCollection_Vector<TopoDS_Edge>& newedges, NCollection_Vector<TopoDS_Edge>& notTaken, double tol, bool *pClosed, double* pMaxGap);
			int  GetMatchTwoPntsPair(const gp_Pnt& b1, const gp_Pnt& e1, const gp_Pnt& b2, const gp_Pnt& e2, double& minDis, double& otherDis);
		
			
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
			XbimWire(IIfcPolyline^ loop, ILogger^ logger);
			XbimWire(IIfcPolyline^ loop, bool attemptClosing, ILogger^ logger);
			XbimWire(IIfcBSplineCurve^ bspline, ILogger^ logger);
			XbimWire(IIfcBSplineCurveWithKnots^ bSpline, ILogger^ logger);
			XbimWire(IIfcRationalBSplineCurveWithKnots^ bSpline, ILogger^ logger);
			XbimWire(IIfcCompositeCurve^ loop, ILogger^ logger);
			XbimWire(IIfcTrimmedCurve^ loop, ILogger^ logger);
			XbimWire(IIfcCurve^ loop, ILogger^ logger);
			XbimWire(IIfcIndexedPolyCurve^ pcurve, ILogger^ logger);
			XbimWire(IIfcCompositeCurveSegment^ compCurveSeg, ILogger^ logger);
			XbimWire(IIfcBoundedCurve^ loop, ILogger^ logger);
			XbimWire(IIfcPolyLoop^ loop, ILogger^ logger);
			XbimWire(IIfcArbitraryClosedProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcArbitraryOpenProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcCenterLineProfileDef^ profile, ILogger^ logger);
			//parametrised profiles
			XbimWire(IIfcProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcDerivedProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcParameterizedProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcCircleProfileDef^ circProfile, ILogger^ logger);
			XbimWire(IIfcRectangleProfileDef^ rectProfile, ILogger^ logger);
			XbimWire(IIfcRoundedRectangleProfileDef^ rectProfile, ILogger^ logger);
			XbimWire(IIfcLShapeProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcUShapeProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcEllipseProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcIShapeProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcZShapeProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcCShapeProfileDef^ profile, ILogger^ logger);
			XbimWire(IIfcTShapeProfileDef^ profile, ILogger^ logger);


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
			virtual IXbimWire^ Trim(double first, double last, double tolerance,ILogger^ logger);

			virtual property double Length {double get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			void FuseColinearSegments(double tolerance, double angleTolerance, ILogger^ logger);
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

			XbimWire^ Trim(XbimVertex^ first, XbimVertex^ last, double tolerance, ILogger^ logger);



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

			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger) override;
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