#pragma once
#include "XbimOccShape.h"
#include "XbimWire.h"
#include "XbimWireSet.h"
#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_FaceError.hxx>
#include "OCC/src/Geom/Geom_Surface.hxx"
 
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
			void Init(IIfcProfileDef^ profile, ILogger^ logger);
			void Init(IIfcArbitraryProfileDefWithVoids^ profile, ILogger^ logger);
			void Init(IIfcCircleHollowProfileDef ^ circProfile, ILogger^ logger);
			void Init(IIfcCompositeProfileDef ^ profile, ILogger^ logger);
			void Init(IIfcDerivedProfileDef ^ profile, ILogger^ logger);
			void Init(IIfcRectangleHollowProfileDef^ rectProfile, ILogger^ logger);
			void Init(IIfcSurface^ surface, ILogger^ logger);
			void Init(IIfcPlane^ plane, ILogger^ logger);
			void Init(IIfcSurfaceOfLinearExtrusion^ sLin, ILogger^ logger);
			void Init(IIfcSurfaceOfRevolution^ sRev, ILogger^ logger);
			void Init(IIfcRectangularTrimmedSurface^ def, ILogger^ logger);
			void Init(IIfcCurveBoundedPlane^ def, ILogger^ logger);
			void Init(IIfcCompositeCurve ^ cCurve, ILogger^ logger);
			void Init(IIfcPolyline ^ pline, ILogger^ logger);
			void Init(IIfcPolyLoop ^ loop, ILogger^ logger);
			void Init(IXbimWire^ wire, ILogger^ logger);
			void Init(IXbimWire^ wire, XbimPoint3D pointOnFace, XbimVector3D faceNormal, ILogger^ logger);
			void Init(IXbimFace^ face, ILogger^ logger);
			void Init(IIfcBSplineSurface ^ surface, ILogger^ logger);
			void Init(IIfcBSplineSurfaceWithKnots ^ surface, ILogger^ logger);
			void Init(IIfcRationalBSplineSurfaceWithKnots ^ surface, ILogger^ logger);
			void Init(IIfcCylindricalSurface ^ surface, ILogger^ logger);
			void Init(double x, double y, double tolerance, ILogger^ logger); 
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
			virtual property XbimPoint3D Location {XbimPoint3D get(); }
#pragma endregion

			property bool IsReversed{bool get(){ return IsValid && pFace->Orientation() == TopAbs_REVERSED; }; }
			
#pragma region constructors
			XbimFace(const TopoDS_Face& face);
			XbimFace(const TopoDS_Face& face, Object^ tag);
			XbimFace(){}; //an invalid empty face
			XbimFace(XbimVector3D normal, ILogger^ logger);
			XbimFace(XbimPoint3D location, XbimVector3D normal, ILogger^ logger);
			XbimFace(IIfcProfileDef^ profile, ILogger^ logger);
			//Builds a face from a Surface
			XbimFace(IIfcSurface^ surface, ILogger^ logger);

			//Builds a face from a Plane
			XbimFace(IIfcPlane ^ plane, ILogger^ logger);
			XbimFace(IIfcSurfaceOfLinearExtrusion ^ sLin, ILogger^ logger);
			XbimFace(IIfcSurfaceOfRevolution ^ sRev, ILogger^ logger);
			XbimFace(IIfcCurveBoundedPlane ^ def, ILogger^ logger);
			XbimFace(IIfcRectangularTrimmedSurface ^ def, ILogger^ logger);
			XbimFace(IIfcCompositeCurve ^ cCurve, ILogger^ logger);
			XbimFace(IIfcPolyline ^ pline, ILogger^ logger);
			XbimFace(IIfcPolyLoop ^ loop, ILogger^ logger);
			XbimFace(IXbimWire^ wire, ILogger^ logger);
			XbimFace(IXbimWire^ wire, XbimPoint3D pointOnface,  XbimVector3D faceNormal, ILogger^ logger);
			XbimFace(IXbimFace^ face, ILogger^ logger);
			XbimFace(IIfcSurface^ surface, XbimWire^ outerBound, IEnumerable<XbimWire^>^ innerBounds, ILogger^ logger);
			XbimFace(IIfcFaceSurface^ surface, XbimWire^ outerBound, IEnumerable<XbimWire^>^ innerBounds, double tolerance, ILogger^ logger);
			XbimFace(IIfcCylindricalSurface ^ surface, ILogger^ logger);
			XbimFace(double x, double y, double tolerance, ILogger^ logger);
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

			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger) override;

};
	}

}