#pragma once
#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimFace.h"
#include "XbimWire.h"
#include "XbimCompound.h"

using namespace System::IO;

namespace Xbim
{
	namespace Geometry
	{

		public ref class XbimOccWriter
		{
			
		public:
			XbimOccWriter();
			static bool Write(const TopoDS_Shape& shape, System::String^ fileName);
			static bool Write(IXbimGeometryObject^ obj, System::String^ filename);
			static bool Write(IXbimGeometryObjectSet^ objects, System::String^ filename);
			static bool Write(IXbimSolidSet^ solidSet, System::String^ filename);
			static bool Write(IXbimSolid^ solid, System::String^ filename);
			static bool Write(IXbimShell^ shell, System::String^ filename);
			static bool Write(IXbimFace^ face, System::String^ filename);
			static bool Write(IXbimWire^ wire, System::String^ filename);
		};
	}
}