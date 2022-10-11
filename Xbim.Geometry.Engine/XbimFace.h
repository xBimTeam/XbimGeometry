#pragma once

#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_FaceError.hxx>
#include <Geom_Surface.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TopTools_DataMapOfIntegerShape.hxx>
#include <vector>
#include "XbimOccShape.h"
#include "XbimWire.h"
#include "XbimWireSet.h"
using namespace System::Collections::Generic;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimFaceV5 : IXbimFace, XbimOccShape
		{
		private:
			
			System::IntPtr ptrContainer;
			virtual property TopoDS_Face* pFace
			{
				TopoDS_Face* get() sealed { return (TopoDS_Face*)ptrContainer.ToPointer(); }
				void set(TopoDS_Face* val)sealed { ptrContainer = System::IntPtr(val); }
			}
			void InstanceCleanup();

			//initialisers
			void Init(IIfcProfileDef^ profile, ILogger^ logger);
			void Init(IIfcArbitraryProfileDefWithVoids^ profile, ILogger^ logger);
			void Init(IIfcCircleHollowProfileDef ^ circProfile, ILogger^ logger);
			void Init(IIfcCompositeProfileDef ^ profile, ILogger^ logger);
			void Init(IIfcDerivedProfileDef ^ profile, ILogger^ logger);
			void Init(IIfcRectangleHollowProfileDef^ rectProfile, ILogger^ logger);
			void Init(IIfcSurface^ surface, ILogger^ logger);
			void Init(IIfcPlane^ plane, ILogger^ logger);
			void Init(IIfcSurfaceOfLinearExtrusion^ sLin,  ILogger^ logger);
			void Init(IIfcSurfaceOfLinearExtrusion^ sLin, bool useWorkArounds,  ILogger^ logger);
			void ReParamCurve(TopoDS_Edge& basisEdge);
			void ReparamBSpline(Handle(Geom_Curve)& curve, const Standard_Real First, const Standard_Real Last);
			TopoDS_Edge ReParamEdge(TopoDS_Edge& basisEdge);
			void Init(IIfcSurfaceOfRevolution^ sRev, ILogger^ logger);
			void Init(IIfcRectangularTrimmedSurface^ def, ILogger^ logger);
			void Init(IIfcCurveBoundedPlane^ def, ILogger^ logger);
			void Init(IIfcCompositeCurve ^ cCurve, ILogger^ logger);
			void Init(IIfcPolyline ^ pline, ILogger^ logger);
			void Init(IIfcPolyLoop ^ loop, ILogger^ logger);
			void Init(IXbimWire^ wire, bool isPlanar, double precision, int entityLabel, ILogger^ logger);
			void Init(IXbimWire^ wire, XbimPoint3D pointOnFace, XbimVector3D faceNormal, ILogger^ logger);
			void Init(IXbimFace^ face, ILogger^ logger);
			void Init(IIfcBSplineSurface ^ surface, ILogger^ logger);
			void Init(IIfcBSplineSurfaceWithKnots ^ surface, ILogger^ logger);
			void Init(IIfcRationalBSplineSurfaceWithKnots ^ surface, ILogger^ logger);
			void Init(IIfcCylindricalSurface ^ surface, ILogger^ logger);
			void Init(double x, double y, double tolerance, ILogger^ logger); 
			void Init(IIfcFace^ face, ILogger^ logger, bool userVertexMap,  TopTools_DataMapOfIntegerShape& vertexMap);
			
		public:
			
			//destructors
			~XbimFaceV5(){ InstanceCleanup(); }
			!XbimFaceV5(){ InstanceCleanup(); }

			//error logging
			static System::String^ GetBuildFaceErrorMessage(BRepBuilderAPI_FaceError err);

#pragma region operators
			
			operator const TopoDS_Face& () { return *pFace; }
			virtual operator const TopoDS_Shape& () override { return *pFace; }

#pragma endregion

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimFaceV5^ left, XbimFaceV5^ right);
			static bool operator !=(XbimFaceV5^ left, XbimFaceV5^ right);
			virtual bool Equals(IXbimFace^ f);
#pragma endregion


#pragma region IXbimFace Interface
			virtual property bool IsValid{bool get() override { return pFace != nullptr; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimFaceType; }; }
			virtual property IXbimWire^ OuterBound{ IXbimWire^ get(){ return OuterWire; } }
			virtual property IXbimWireSet^ InnerBounds{IXbimWireSet^ get(){ return InnerWires; } }
			virtual property IXbimWireSet^ Bounds{IXbimWireSet^ get(){ return Wires; } }
			virtual property double Area { double get(); }
			virtual property double Perimeter { double get(); }
			virtual property bool IsPlanar{bool get(); }
			virtual property bool IsPolygonal{bool get(); }
			///The topological normal of the face
			virtual property XbimVector3D Normal{ XbimVector3D get(); }
			virtual property XbimRect3D BoundingBox{XbimRect3D get() override; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual property bool IsQuadOrTriangle{bool get(); }
			virtual void SaveAsBrep(System::String^ fileName);
			virtual property XbimPoint3D Location {XbimPoint3D get(); }
#pragma endregion

			property bool IsReversed{bool get(){ return IsValid && pFace->Orientation() == TopAbs_REVERSED; }; }
			
#pragma region constructors
			XbimFaceV5(const TopoDS_Face& face);
			XbimFaceV5(const TopoDS_Face& face, Object^ tag);
			XbimFaceV5(){}; //an invalid empty face
			XbimFaceV5(XbimVector3D normal, ILogger^ logger);
			XbimFaceV5(XbimPoint3D location, XbimVector3D normal, ILogger^ logger);
			XbimFaceV5(IIfcProfileDef^ profile, ILogger^ logger);
			//Builds a face from a Surface
			XbimFaceV5(IIfcSurface^ surface, ILogger^ logger);

			//Builds a face from a Plane
			XbimFaceV5(IIfcPlane ^ plane, ILogger^ logger);
			XbimFaceV5(IIfcSurfaceOfLinearExtrusion ^ sLin, bool useWorkArounds, ILogger^ logger);
			XbimFaceV5(IIfcSurfaceOfLinearExtrusion^ sLin,  ILogger^ logger);
			XbimFaceV5(IIfcSurfaceOfRevolution ^ sRev, ILogger^ logger);
			XbimFaceV5(IIfcCurveBoundedPlane ^ def, ILogger^ logger);
			XbimFaceV5(IIfcRectangularTrimmedSurface ^ def, ILogger^ logger);
			XbimFaceV5(IIfcCompositeCurve ^ cCurve, ILogger^ logger);
			XbimFaceV5(IIfcPolyline ^ pline,  ILogger^ logger);
			XbimFaceV5(IIfcPolyLoop ^ loop, ILogger^ logger);
			XbimFaceV5(IXbimWire^ wire, bool isPlanar, double precision, int entityLabel,  ILogger^ logger);
			XbimFaceV5(IXbimWire^ wire, XbimPoint3D pointOnface,  XbimVector3D faceNormal, ILogger^ logger);
			XbimFaceV5(IIfcSurface^ surface, XbimWireV5^ outerBound, IEnumerable<XbimWireV5^>^ innerBounds, ILogger^ logger);
			static void PutEdgeOnFace(const TopoDS_Edge& Edg, const TopoDS_Face& Fac);
			XbimFaceV5(IIfcFaceSurface^ surface, XbimWireV5^ outerBound, IEnumerable<XbimWireV5^>^ innerBounds, double tolerance, ILogger^ logger);
			bool CheckInside();
			XbimFaceV5(IIfcCylindricalSurface ^ surface, ILogger^ logger);
			XbimFaceV5(double x, double y, double tolerance, ILogger^ logger);
			XbimFaceV5(IIfcFace ^ face, ILogger^ logger, bool useVertexMap, TopTools_DataMapOfIntegerShape & vertexMap);
			
#pragma endregion

#pragma region Internal Properties
			property XbimWireSet^  Wires{XbimWireSet^ get(); }
			property XbimWireSet^  InnerWires{XbimWireSet^ get(); }
			property XbimWireV5^  OuterWire{XbimWireV5^ get(); }
#pragma endregion	

#pragma region Methods
			//moves the face to the new position
			void Move(IIfcAxis2Placement3D^ position);
			void Move(gp_Trsf transform);
			void Translate(XbimVector3D translation);
			void Reverse();
			bool Add(IXbimWire^ innerWire);
			XbimPoint3D PointAtParameters(double u, double v);
			Handle(Geom_Surface) GetSurface();
			XbimVector3D NormalAt(double u, double v);
			void SetLocation(TopLoc_Location loc);
			static bool RemoveDuplicatePoints(TColgp_SequenceOfPnt& polygon, std::vector<int> handles, bool closed, double tol);
			static bool RemoveDuplicatePoints(TColgp_SequenceOfPnt& polygon,  bool closed, double tol);
#pragma endregion



			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;


			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;

			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger) override;

};
	}

}