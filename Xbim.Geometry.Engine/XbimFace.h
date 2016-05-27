#pragma once
#include "XbimOccShape.h"
#include "XbimWire.h"
#include "XbimWireSet.h"
#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_FaceError.hxx>
#include "XbimWireSet.h"
#include "OCC/src/Geom/Geom_Surface.hxx"

using namespace System;
using namespace System::Collections::Generic;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimFace : IXbimFace, XbimOccShape
		{
		private:
			
			IntPtr ptrContainer;
			virtual property TopoDS_Face* pFace
			{
				TopoDS_Face* get() sealed { return (TopoDS_Face*)ptrContainer.ToPointer(); }
				void set(TopoDS_Face* val)sealed { ptrContainer = IntPtr(val); }
			}
			void InstanceCleanup();

			//initialisers
			void Init(IIfcProfileDef^ profile);
			void Init(IIfcArbitraryProfileDefWithVoids^ profile);
			void Init(IIfcCircleHollowProfileDef ^ circProfile);
			void Init(IIfcCompositeProfileDef ^ profile);
			void Init(IIfcDerivedProfileDef ^ profile);
			void Init(IIfcRectangleHollowProfileDef^ rectProfile);
			void Init(IIfcSurface^ surface);
			void Init(IIfcPlane^ plane);
			void Init(IIfcSurfaceOfLinearExtrusion^ sLin);
			void Init(IIfcSurfaceOfRevolution^ sRev);
			void Init(IIfcRectangularTrimmedSurface^ def);
			void Init(IIfcCurveBoundedPlane^ def);
			void Init(IIfcCompositeCurve ^ cCurve);
			void Init(IIfcPolyline ^ pline);
			void Init(IIfcPolyLoop ^ loop);
			void Init(IXbimWire^ wire);
			void Init(IXbimWire^ wire, XbimPoint3D pointOnFace, XbimVector3D faceNormal);
			void Init(IXbimFace^ face);
			void Init(IIfcBSplineSurface ^ surface);
			void Init(IIfcBSplineSurfaceWithKnots ^ surface);
			void Init(IIfcRationalBSplineSurfaceWithKnots ^ surface);
			void Init(IIfcCylindricalSurface ^ surface);
			void Init(double x, double y, double tolerance); 
		public:
			
			//destructors
			~XbimFace(){ InstanceCleanup(); }
			!XbimFace(){ InstanceCleanup(); }

			//error logging
			static String^ GetBuildFaceErrorMessage(BRepBuilderAPI_FaceError err);

#pragma region operators
			
			operator const TopoDS_Face& () { return *pFace; }
			virtual operator const TopoDS_Shape& () override { return *pFace; }

#pragma endregion

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimFace^ left, XbimFace^ right);
			static bool operator !=(XbimFace^ left, XbimFace^ right);
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
			virtual void SaveAsBrep(String^ fileName);
#pragma endregion

			property bool IsReversed{bool get(){ return IsValid && pFace->Orientation() == TopAbs_REVERSED; }; }
			property XbimPoint3D Location{XbimPoint3D get(); }
#pragma region constructors
			XbimFace(const TopoDS_Face& face);
			XbimFace(const TopoDS_Face& face, Object^ tag);
			XbimFace(){}; //an invalid empty face
			XbimFace(XbimVector3D normal);
			XbimFace(XbimPoint3D location, XbimVector3D normal);
			XbimFace(IIfcProfileDef^ profile);
			//Builds a face from a Surface
			XbimFace(IIfcSurface^ surface);

			//Builds a face from a Plane
			XbimFace(IIfcPlane ^ plane);
			XbimFace(IIfcSurfaceOfLinearExtrusion ^ sLin);
			XbimFace(IIfcSurfaceOfRevolution ^ sRev);
			XbimFace(IIfcCurveBoundedPlane ^ def);
			XbimFace(IIfcRectangularTrimmedSurface ^ def);
			XbimFace(IIfcCompositeCurve ^ cCurve);
			XbimFace(IIfcPolyline ^ pline);
			XbimFace(IIfcPolyLoop ^ loop);
			XbimFace(IXbimWire^ wire);
			XbimFace(IXbimWire^ wire, XbimPoint3D pointOnface,  XbimVector3D faceNormal);
			XbimFace(IXbimFace^ face);
			XbimFace(IIfcSurface^ surface, XbimWire^ outerBound, IEnumerable<XbimWire^>^ innerBounds);
			XbimFace(IIfcFaceSurface^ surface, XbimWire^ outerBound, IEnumerable<XbimWire^>^ innerBounds, double tolerance);
			XbimFace(IIfcCylindricalSurface ^ surface);
			XbimFace(double x, double y, double tolerance);
#pragma endregion

#pragma region Internal Properties
			property XbimWireSet^  Wires{XbimWireSet^ get(); }
			property XbimWireSet^  InnerWires{XbimWireSet^ get(); }
			property XbimWire^  OuterWire{XbimWire^ get(); }
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
#pragma endregion



			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;


			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;

			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement) override;

};
	}

}