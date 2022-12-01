#include "GeometryConverterFactory.h"
#include "../Services/ModelGeometryService.h"
#include "../XbimGeometryCreator.h"
#include "../XbimGeometryCreatorV6.h"
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
			IXbimGeometryEngine^ GeometryConverterFactory::CreateGeometryEngineV6(IModel^ model, ILoggerFactory^ loggerFactory)
			{
				return gcnew XbimGeometryCreatorV6(model, loggerFactory);
				
			}
			IXbimGeometryEngine^ GeometryConverterFactory::CreateGeometryEngine(XGeometryEngineVersion version, IModel^ model, ILoggerFactory^ loggerFactory)
			{
				switch (version)
				{
				case Xbim::Geometry::Abstractions::XGeometryEngineVersion::V5:
					return CreateGeometryEngineV5(model, loggerFactory);
				case Xbim::Geometry::Abstractions::XGeometryEngineVersion::V6:
					return CreateGeometryEngineV6(model, loggerFactory);
				default:
					throw gcnew System::ArgumentOutOfRangeException("Invalid Geometry Engine Version");
				}
			}
		}
	}
}