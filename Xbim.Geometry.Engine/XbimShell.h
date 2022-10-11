#pragma once

#include <TopoDS_Shell.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_DataMapOfIntegerShape.hxx>
#include "XbimOccShape.h"
#include "XbimWire.h"
#include "XbimFace.h"
#include "XbimEdge.h"
using namespace System::Collections::Generic;
using namespace Xbim::Common::Geometry;

namespace Xbim
{
	namespace Geometry
	{

		ref class XbimShellV5 : IXbimShell, XbimOccShape
		{
		private:
			
			System::IntPtr ptrContainer;
			virtual property TopoDS_Shell* pShell
			{
				TopoDS_Shell* get() sealed { return (TopoDS_Shell*)ptrContainer.ToPointer(); }
				void set(TopoDS_Shell* val)sealed { ptrContainer = System::IntPtr(val); }
			}
			void InstanceCleanup();
			void Init(IIfcOpenShell^ openShell, ILogger^ logger);
			void Init(IIfcConnectedFaceSet^ faceset, ILogger^ logger);
			void Init(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^ logger);
		public:
			//Constructors
			XbimShellV5();
			XbimShellV5(const TopoDS_Shell& shell);
			XbimShellV5(const TopoDS_Shell& shell, Object^ tag);
			XbimShellV5(IIfcOpenShell^ openShell, ILogger^ logger);
			XbimShellV5(IIfcConnectedFaceSet^ faceset, ILogger^ logger);
			XbimShellV5(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^ logger);
			//destructors
			~XbimShellV5(){ InstanceCleanup(); }
			!XbimShellV5(){ InstanceCleanup(); }

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimShellV5^ left, XbimShellV5^ right);
			static bool operator !=(XbimShellV5^ left, XbimShellV5^ right);
			virtual bool Equals(IXbimShell^ s);
#pragma endregion

#pragma region IXbimShell Interface	
			virtual property bool IsEmpty {bool get(); }
			virtual property bool IsValid{bool get() override { return pShell != nullptr && !pShell->IsNull(); }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimShellType; }; }
			virtual property IXbimFaceSet^ Faces{ IXbimFaceSet^ get(); }
			virtual property IXbimEdgeSet^ Edges{ IXbimEdgeSet^ get(); }
			virtual property IXbimVertexSet^ Vertices{IXbimVertexSet^ get(); }
			virtual property XbimRect3D BoundingBox{XbimRect3D get() override; }
			virtual property bool IsClosed{bool get(); }
			virtual property double SurfaceArea { double get(); }
			virtual property bool IsPolyhedron { bool get(); }
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Union(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolidSet^ solids, double tolerance, ILogger^ logger);
			virtual IXbimGeometryObjectSet^ Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger);
			virtual IXbimFaceSet^ Section(IXbimFace^ face, double tolerance, ILogger^ logger);
			
			virtual IXbimSolid^ MakeSolid();
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual property bool HasValidTopology{bool get(); }
			virtual bool CanCreateSolid(){ return IsClosed; };
			virtual IXbimSolid^ CreateSolid(){ return MakeSolid(); };
			virtual void SaveAsBrep(System::String^ fileName);
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
			virtual XbimGeometryObject ^ Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger) override;
				virtual void Move(TopLoc_Location loc);
		};
	}
}