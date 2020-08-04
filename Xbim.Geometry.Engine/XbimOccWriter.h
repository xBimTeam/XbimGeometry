#pragma once
#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimFace.h"
#include "XbimWire.h"
#include "XbimCompound.h"
using namespace System;
using namespace System::IO;

namespace Xbim
{
	namespace Geometry
	{

		public ref class XbimOccWriter
		{
			
		public:
			XbimOccWriter();
			static bool Write(const TopoDS_Shape& shape, String^ fileName);
			static bool Write(IXbimGeometryObject^ obj, String^ filename);
			static bool Write(IXbimGeometryObjectSet^ objects, String^ filename);
			static bool Write(IXbimSolidSet^ solidSet, String^ filename);
			static bool Write(IXbimSolid^ solid, String^ filename);
			static bool Write(IXbimShell^ shell, String^ filename);
			static bool Write(IXbimFace^ face, String^ filename);
			static bool Write(IXbimWire^ wire, String^ filename);
		};
	}
}