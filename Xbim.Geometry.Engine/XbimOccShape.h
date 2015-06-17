#pragma once
#include "XbimGeometryObject.h"
#include <TopoDS_Shape.hxx>
#include <BRepBuilderAPI_Copy.hxx>
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{
		

		ref class XbimOccShape abstract : XbimGeometryObject
		{
		
			
		public:
			static void WriteIndex(BinaryWriter^ bw, UInt32 index, UInt32 maxInt);
			XbimOccShape();
			//operators
			virtual operator const TopoDS_Shape& () abstract;
			void WriteTriangulation(TextWriter^ textWriter, double tolerance, double deflection, double angle);
			void WriteTriangulation(BinaryWriter^ binaryWriter, double tolerance, double deflection, double angle);
			virtual property bool IsSet{bool get() override { return false; }; }
			
		};
	}
}

