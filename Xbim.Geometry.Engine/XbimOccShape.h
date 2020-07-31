#pragma once
#include "XbimGeometryObject.h"
#include <TopoDS_Shape.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <OSD_Timer.hxx>

using namespace System::IO;
using namespace System::Collections::Generic;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;


namespace Xbim
{
	namespace Geometry
	{
#pragma managed(push,off)
		struct Error
		{
			explicit Error(std::string const& message) : message_(message) { }
			char const* what() const throw() { return message_.c_str(); }

		private:
			std::string message_;
		};
#pragma managed(pop)

		ref class XbimOccShape abstract : XbimGeometryObject
		{
		
		
			
		public:
			static void WriteIndex(BinaryWriter^ bw, UInt32 index, UInt32 maxInt);
			XbimOccShape();
			//operators
			virtual operator const TopoDS_Shape& () abstract;
			void WriteTriangulation(TextWriter^ textWriter, double tolerance, double deflection, double angle);
			void WriteTriangulation(BinaryWriter^ binaryWriter, double tolerance, double deflection, double angle);
			void WriteTriangulation(IXbimMeshReceiver^ mesh, double tolerance, double deflection, double angle);
			virtual property bool IsSet{bool get() override { return false; }; }
			virtual XbimGeometryObject^ Transformed(IIfcCartesianTransformationOperator ^transformation) abstract;
			virtual XbimGeometryObject^ Moved(IIfcPlacement ^placement) abstract;
			virtual XbimGeometryObject^ Moved(IIfcObjectPlacement ^objectPlacement, ILogger^ logger) abstract;
			virtual void Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle)
			{
				WriteTriangulation(mesh, precision, deflection, angle);
			}
			// Inherited via XbimGeometryObject

			
			
		};
	}
}

