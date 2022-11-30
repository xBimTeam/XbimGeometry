#include "GeometryConverterFactory.h"
#include "../Services/ModelGeometryService.h"
#include "../XbimGeometryCreator.h"

using namespace Xbim::Geometry::Services;
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXbimGeometryEngine^ GeometryConverterFactory::CreateGeometryEngineV5(IModel^ model, ILoggerFactory^ loggerFactory)
			{
				
				return gcnew XbimGeometryCreator(model, loggerFactory);
			}

			IXModelGeometryService^ GeometryConverterFactory::CreateModelGeometryService(IModel^ model, ILoggerFactory^ loggerFactory)
			{
				return gcnew ModelGeometryService(model, loggerFactory);
			}
		}
	}
}