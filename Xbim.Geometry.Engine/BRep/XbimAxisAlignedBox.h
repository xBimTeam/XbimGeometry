#pragma once
#pragma once
#include "../XbimHandle.h"
#include<Bnd_Box.hxx>

using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimAxisAlignedBox : XbimHandle<Bnd_Box>, IXAxisAlignedBoundingBox
			{
			public:
				XbimAxisAlignedBox(const Bnd_Box& box) : XbimHandle(new Bnd_Box(box)) { };
				XbimAxisAlignedBox(Bnd_Box* pBox) : XbimHandle(pBox) { };
				virtual property IXPoint^ CornerMin {IXPoint^ get(); };
				virtual property IXPoint^ CornerMax {IXPoint^ get(); };
				virtual property String^ Json {String^ get(); };
				virtual property double LenX {double get(); };
				virtual property double LenY {double get(); };
				virtual property double LenZ {double get(); };
				virtual property double Gap {double get(); };
			};

		}
	}
}
