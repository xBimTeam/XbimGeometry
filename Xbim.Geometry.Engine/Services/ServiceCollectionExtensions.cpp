#pragma warning(disable:4691) //turn off netstandard 2.0 library warnings, its in the same module
#include "ServiceCollectionExtensions.h"
#include "../storage/BRepDocumentManager.h"

IServiceCollection^ Microsoft::Extensions::DependencyInjection::ServiceCollectionExtensions::AddGeometryEngineServices(IServiceCollection^ services)
{
	Microsoft::Extensions::DependencyInjection::ServiceCollectionServiceExtensions::AddSingleton<IXGeometryConverterFactory^, GeometryConverterFactory^>(services);
	Microsoft::Extensions::DependencyInjection::ServiceCollectionServiceExtensions::AddSingleton<IXBRepDocumentManager^, Xbim::Geometry::Storage::BRepDocumentManager^>(services);
	return services;
};

