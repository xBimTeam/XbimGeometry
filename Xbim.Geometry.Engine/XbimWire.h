#pragma once
#include "XbimOccShape.h"
#include "XbimEdge.h"
#include "XbimVertex.h"
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <TopoDS_Edge.hxx>
#include <vector>
#include <NCollection_Vector.hxx>
#include "XbimConstraints.h"
#include <BRepBuilderAPI_MakeWire.hxx>

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
			void ModifyWireAddEdge(TopoDS_Wire& resultWire,
				const TopoDS_Edge& EE,
				const TopoDS_Vertex& edgeVertexToJoin, gp_Pnt edgePointToJoin, 
				const TopoDS_Vertex& nextEdgeVertex, 
				const TopoDS_Vertex& wireVertexToJoin, gp_Pnt wirePointToJoin, 
				double distance);
#pragma region initialisation functions

			void Init(double precision);
			
			void Init(IIfcCurve^ loop, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcCompositeCurve^ compCurve, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcCompositeCurveSegment^ compCurveSeg, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcPolyline^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcIndexedPolyCurve ^ pCurve, ILogger ^ logger, XbimConstraints constraints);
			void Init(IIfcPolyLoop^ loop, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcArbitraryClosedProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcArbitraryOpenProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcCenterLineProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			//parametrised profiles
			void Init(IIfcProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcDerivedProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcParameterizedProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcCircleProfileDef^ circProfile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcRectangleProfileDef^ rectProfile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcRoundedRectangleProfileDef^ rectProfile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcLShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcUShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcEllipseProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcIShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcZShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcCShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			void Init(IIfcTShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			//constructs a rectangle wire with the bottom left corner at 0,0,0, top right at x,y,0
			void Init(double x, double y, double tolerance, bool centre);
#pragma endregion

			//helpers
			static void AddNewellPoint(const gp_Pnt& previous, const gp_Pnt& current, double & x, double & y, double & z);
			bool AreEdgesC1(const TopoDS_Edge& e1, const TopoDS_Edge& e2, double precision, double angularTolerance);
			bool SortEdgesForWire(const NCollection_Vector<TopoDS_Edge>& oldedges, NCollection_Vector<TopoDS_Edge>& newedges, NCollection_Vector<TopoDS_Edge>& notTaken, double tol, bool *pClosed, double* pMaxGap);
			int  GetMatchTwoPntsPair(const gp_Pnt& b1, const gp_Pnt& e1, const gp_Pnt& b2, const gp_Pnt& e2, double& minDis, double& otherDis);
			
			
		public:
			// this can throw an exception in debug, if the wire is nonsense (e.g. line) and should just be dropped
			// in release it might return an invalid normal with Nan components that needs to be checked, using XbimConvert::IsInvalid
			static gp_Dir NormalDir(const TopoDS_Wire& wire);
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
			XbimWire(IIfcCurve^ loop, ILogger^ logger, XbimConstraints constraints);
			
			//special case for building a composite curve as a wire and not a single edge
			XbimWire(IIfcCompositeCurve^ compCurve, ILogger^ logger, XbimConstraints constraints);
			//srl need to revisit this, the sense is wrong for trimmed curves, really it should not be supported at all as the segment is not a curve
			XbimWire(IIfcCompositeCurveSegment^ compCurveSeg, ILogger^ logger, XbimConstraints constraints);

			//Creates a wire of individual edges for each IfcPolyline segment, use XbimCurve for a single bspline edge
			XbimWire(IIfcPolyline^ profile, ILogger^ logger, XbimConstraints constraints);
			//Creates a wire of individual edges for each IfcIndexedPolyCurve segment, use XbimCurve for a single bspline edge
			XbimWire(IIfcIndexedPolyCurve^ profile, ILogger^ logger, XbimConstraints constraints);

			XbimWire(IIfcPolyLoop^ loop, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcArbitraryClosedProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcArbitraryOpenProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcCenterLineProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			//parametrised profiles
			XbimWire(IIfcProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcDerivedProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcParameterizedProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcCircleProfileDef^ circProfile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcRectangleProfileDef^ rectProfile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcRoundedRectangleProfileDef^ rectProfile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcLShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcUShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcEllipseProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcIShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcZShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcCShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);
			XbimWire(IIfcTShapeProfileDef^ profile, ILogger^ logger, XbimConstraints constraints);


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
			//returns the normal of the loop using the Newell's normal algorithm. WARNING: Always check success via the IsInvalid() method of returned instance.
			virtual property XbimVector3D Normal {XbimVector3D get(); }
			virtual property bool IsClosed {bool get() { return IsValid && pWire->Closed() == Standard_True; }; }
			virtual property bool IsPlanar {bool get(); }
			virtual property XbimPoint3D Start {XbimPoint3D get(); }
			virtual property XbimPoint3D End {XbimPoint3D get(); }
			virtual IXbimWire^ Trim(double first, double last, double tolerance,ILogger^ logger);

			virtual property XbimPoint3D BaryCentre {XbimPoint3D get(); }
			virtual property double Length {double get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			void FuseColinearSegments(double tolerance, double angleTolerance, ILogger^ logger);
			virtual property double Area {double get(); }
			virtual property double MaxTolerance {double get() {return IsValid ? BRep_Tool::MaxTolerance(*pWire, TopAbs_VERTEX) : 0; } }
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
			virtual property gp_Pnt StartPoint {gp_Pnt get(); }
			virtual property gp_Pnt EndPoint {gp_Pnt get(); }
			virtual property  TopoDS_Vertex StartVertex { TopoDS_Vertex get(); }
			virtual property  TopoDS_Vertex EndVertex { TopoDS_Vertex get(); }

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