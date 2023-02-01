#pragma warning(disable:4691) //turn off netstandard 2.0 library warnings, its in the same module
#include "ServiceCollectionExtensions.h"
#include "../storage/BRepDocumentManager.h"
#include "../Primitives/GeometryPrimitives.h"
#include "ShapeService.h"
using namespace Xbim::Geometry::DependencyInjection;

IServiceCollection^ ServiceCollectionExtensions::AddGeometryEngineServices(IServiceCollection^ services)
{
	ServiceCollectionServiceExtensions::AddSingleton<IXGeometryConverterFactory^, GeometryConverterFactory^>(services);
	ServiceCollectionServiceExtensions::AddSingleton<IXBRepDocumentManager^, Xbim::Geometry::Storage::BRepDocumentManager^>(services);
	ServiceCollectionServiceExtensions::AddSingleton<IXGeometryPrimitives^, Xbim::Geometry::Primitives::GeometryPrimitives^>(services);
	ServiceCollectionServiceExtensions::AddSingleton<IXShapeService^, Xbim::Geometry::Services::ShapeService^>(services);
	return services;
};

