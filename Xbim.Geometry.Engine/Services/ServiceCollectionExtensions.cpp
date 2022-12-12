#pragma warning(disable:4691) //turn off netstandard 2.0 library warnings, its in the same module
#include "ServiceCollectionExtensions.h"
#include "../Storage/BRepDocumentManager.h"
using namespace Microsoft::Extensions::DependencyInjection;
using namespace Xbim::Geometry::Storage;
IServiceCollection^ ServiceCollectionExtensions::AddGeometryEngineServices(IServiceCollection^ services)
{
	::ServiceCollectionServiceExtensions::AddSingleton<IXGeometryConverterFactory^, GeometryConverterFactory^>(services);
	::ServiceCollectionServiceExtensions::AddSingleton<IXBRepDocumentManager^, BRepDocumentManager^> (services);
	return services;
};

