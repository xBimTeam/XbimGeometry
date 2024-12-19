#pragma once
#pragma warning( disable : 4691 )

#include "../XbimHandle.h"
#include "../BRep/XShape.h"
#include <BinTools.hxx>
#include <array>
#include <istream>

using namespace Xbim::Geometry::Abstractions;


namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			public ref class ShapeBinarySerializer : IXShapeBinarySerializer
			{
			public:
				ShapeBinarySerializer();
				virtual cli::array<System::Byte>^ ToArray(IXShape^ shape, bool withTriangles, bool withNormals);
				virtual IXShape^ FromArray(cli::array<System::Byte>^ bytes);
			};

		}
	}
}