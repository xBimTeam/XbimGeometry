#pragma warning(disable:4691) //turn off netstandard 2.0 library warnings, its in the same module
#include "ServiceCollectionExtensions.h"
#include "../storage/BRepDocumentManager.h"
#include "../Primitives/GeometryPrimitives.h"
#include "ShapeService.h"
using namespace Xbim::Geometry::DependencyInjection;

IServiceCollection^ Xbim::Geometry::DependencyInjection::ServiceCollectionExtensions::AddGeometryEngineServices(IServiceCollection^ services)
{
	ServiceCollectionDescriptorExtensions::TryAddSingleton<IXGeometryConverterFactory^, GeometryConverterFactory^>(services);
	ServiceCollectionDescriptorExtensions::TryAddSingleton<IXBRepDocumentManager^, Xbim::Geometry::Storage::BRepDocumentManager^>(services);
	ServiceCollectionDescriptorExtensions::TryAddSingleton<IXGeometryPrimitives^, Xbim::Geometry::Primitives::GeometryPrimitives^>(services);
	ServiceCollectionDescriptorExtensions::TryAddSingleton<IXShapeService^, Xbim::Geometry::Services::ShapeService^>(services);
	return services;
};

