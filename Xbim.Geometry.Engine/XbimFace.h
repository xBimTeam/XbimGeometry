#pragma once
#include "XbimOccShape.h"
#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_FaceError.hxx>

using namespace System;
using namespace System::Collections::Generic;
using namespace XbimGeometry::Interfaces;
using namespace Xbim::Ifc2x3::ProfileResource;
using namespace Xbim::Ifc2x3::GeometryResource;
using namespace Xbim::Ifc2x3::TopologyResource;
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
			void Init(IfcProfileDef^ profile);
			void Init(IfcArbitraryProfileDefWithVoids^ profile);
			void Init(IfcCircleHollowProfileDef ^ circProfile);
			void Init(IfcRectangleHollowProfileDef^ rectProfile);
			void Init(IfcSurface^ surface);
			void Init(IfcPlane^ plane);
			void Init(IfcSurfaceOfLinearExtrusion^ sLin);
			void Init(IfcSurfaceOfRevolution^ sRev);
			void Init(IfcRectangularTrimmedSurface^ def);
			void Init(IfcCurveBoundedPlane^ def);
			void Init(IfcCompositeCurve ^ cCurve);
			void Init(IfcPolyline ^ pline);
			void Init(IfcPolyLoop ^ loop);
			void Init(IXbimWire^ wire);
			void Init(IXbimWire^ wire, XbimPoint3D pointOnFace, XbimVector3D faceNormal);
			void Init(IXbimFace^ face);
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
			virtual property IXbimWire^ OuterBound{ IXbimWire^ get(); }
			virtual property IXbimWireSet^ InnerBounds{IXbimWireSet^ get(); }
			virtual property double Area { double get(); }
			virtual property double Perimeter { double get(); }
			virtual property bool IsPlanar{bool get(); }
			///The topological normal of the face
			virtual property XbimVector3D Normal{ XbimVector3D get(); }
			virtual property XbimRect3D BoundingBox{XbimRect3D get() override; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
#pragma endregion

			property bool IsReversed{bool get(){ return IsValid && pFace->Orientation() == TopAbs_REVERSED; }; }
			property XbimPoint3D Location{XbimPoint3D get(); }
#pragma region constructors
			XbimFace(const TopoDS_Face& face);
			XbimFace(){}; //an invalid empty face
			XbimFace(XbimVector3D normal);
			XbimFace(XbimPoint3D location, XbimVector3D normal);
			XbimFace(IfcProfileDef^ profile);
			//Builds a face from a Surface
			XbimFace(IfcSurface ^ surface);
			//Builds a face from a Plane
			XbimFace(IfcPlane ^ plane);
			XbimFace(IfcSurfaceOfLinearExtrusion ^ sLin);
			XbimFace(IfcSurfaceOfRevolution ^ sRev);
			XbimFace(IfcCurveBoundedPlane ^ def);
			XbimFace(IfcRectangularTrimmedSurface ^ def);
			XbimFace(IfcCompositeCurve ^ cCurve);
			XbimFace(IfcPolyline ^ pline);
			XbimFace(IfcPolyLoop ^ loop);
			XbimFace(IXbimWire^ wire);
			XbimFace(IXbimWire^ wire, XbimPoint3D pointOnface,  XbimVector3D faceNormal);
			XbimFace(IXbimFace^ face);
			XbimFace(double x, double y, double tolerance);
#pragma endregion

			
#pragma region Methods
			//moves the face to the new position
			void Move(IfcAxis2Placement3D^ position);
			void Translate(XbimVector3D translation);
			void Reverse();
			bool Add(IXbimWire^ innerWire);
			XbimPoint3D PointAtParameters(double u, double v);
#pragma endregion


		};
	}

}