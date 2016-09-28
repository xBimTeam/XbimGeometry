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
			bool Write(const TopoDS_Shape& shape, String^ fileName);
			bool Write(IXbimGeometryObject^ obj, String^ filename);
			bool Write(IXbimGeometryObjectSet^ objects, String^ filename);
			bool Write(IXbimSolidSet^ solidSet, String^ filename);
			bool Write(IXbimSolid^ solid, String^ filename);
			bool Write(IXbimShell^ shell, String^ filename);
			bool Write(IXbimFace^ face, String^ filename);
			bool Write(IXbimWire^ wire, String^ filename);
		};
	}
}