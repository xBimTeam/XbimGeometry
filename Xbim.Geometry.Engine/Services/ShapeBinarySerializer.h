#pragma once

#include "../XbimHandle.h"
#include "../BRep/XShape.h"
#include <BinTools.hxx>
#include "../Services/ModelGeometryService.h"

using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			ref class ModelGeometryService;
		}
	}
}


namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			public ref class ShapeBinarySerializer : IXShapeBinarySerializer
			{
			private:
				IXLoggingService^ _loggingService;
				Xbim::Geometry::Services::ModelGeometryService^ _modelService;
				Object^ _lockObject = gcnew Object();
			public:
				ShapeBinarySerializer(Xbim::Geometry::Services::ModelGeometryService^ modelService);
				virtual array<System::Byte>^ ToArray(IXShape^ shape, bool withTriangles, bool withNormals);
				virtual IXShape^ FromArray(array<System::Byte>^ bytes);
			};

		}
	}
}