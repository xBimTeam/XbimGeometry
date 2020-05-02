#pragma once

using namespace System;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services {
			public ref class BRepService
			{
			private:
				
				
				public:
					int Assemble(IIfcCompositeCurve^ compCurve);
			};
		}
	}
}
