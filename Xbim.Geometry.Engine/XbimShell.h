#pragma once
#include "XbimOccShape.h"
#include "XbimWire.h"
#include "XbimFace.h"
#include "XbimEdge.h"
#include <TopoDS_Shell.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapOfIntegerShape.hxx>

using namespace System::Collections::Generic;
using namespace Xbim::Common::Geometry;

namespace Xbim
{
	namespace Geometry
	{

		ref class XbimShell : IXbimShell, XbimOccShape
		{
		private:
			
			IntPtr ptrContainer;
			virtual property TopoDS_Shell* pShell
			{
				TopoDS_Shell* get() sealed { return (TopoDS_Shell*)ptrContainer.ToPointer(); }
				void set(TopoDS_Shell* val)sealed { ptrContainer = IntPtr(val); }
			}
			void InstanceCleanup();
			void Init(IIfcOpenShell^ openShell);
			void Init(IIfcConnectedFaceSet^ faceset);
			void Init(IIfcSurfaceOfLinearExtrusion^ linExt);
		public:
			//Constructors
			XbimShell();
			XbimShell(const TopoDS_Shell& shell);
			XbimShell(const TopoDS_Shell& shell, Object^ tag);
			XbimShell(IIfcOpenShell^ openShell);
			XbimShell(IIfcConnectedFaceSet^ faceset);
			XbimShell(IIfcSurfaceOfLinearExtrusion^ linExt);
			//destructors
			~XbimShell(){ InstanceCleanup(); }
			!XbimShell(){ InstanceCleanup(); }

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimShell^ left, XbimShell^ right);
			static bool operator !=(XbimShell^ left, XbimShell^ right);
			virtual bool Equals(IXbimShell^ s);
#pragma endregion

#pragma region IXbimShell Interface		
			virtual property bool IsValid{bool get() override { return pShell != nullptr; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimShellType; }; }
			virtual property IXbimFaceSet^ Faces{ IXbimFaceSet^ get(); }
			virtual property IXbimEdgeSet^ Edges{ IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices{IXbimVertexSet^ get(); }
			virtual property XbimRect3D BoundingBox{XbimRect3D get() override; }
			virtual property bool IsClosed{bool get(); }
			virtual property double SurfaceArea { double get(); }
			virtual property bool IsPolyhedron { bool get(); }
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolid^ solid, double tolerance);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolid^ solid, double tolerance);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolidSet^ solids, double tolerance);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolid^ solid, double tolerance);
			virtual IXbimFaceSet^ Section(IXbimFace^ face, double tolerance);
			
			virtual IXbimSolid^ MakeSolid();
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual property bool HasValidTopology{bool get(); }
			virtual bool CanCreateSolid(){ return IsClosed; };
			virtual IXbimSolid^ CreateSolid(){ return MakeSolid(); };
			virtual void SaveAsBrep(String^ fileName);
#pragma endregion
			
#pragma region operators
			operator const TopoDS_Shell& () { return *pShell; }
			virtual operator const TopoDS_Shape& () override { return *pShell; }
#pragma endregion

			//change the direction of the loop
			void Reverse(); 
			//if the shell is closed make sure it is facing the correct way
			void Orientate();
			void FixTopology();

			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Transformed(IIfcCartesianTransformationOperator ^ transformation) override;

			// Inherited via XbimOccShape
			virtual XbimGeometryObject ^ Moved(IIfcPlacement ^ placement) override;
			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement) override;
				virtual void Move(TopLoc_Location loc);
		};
	}
}