#pragma once
#include "../Factories//GeometryConverterFactory.h"

using namespace Microsoft::Extensions::DependencyInjection;
using namespace Xbim::Geometry::Factories;
 
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace DependencyInjection
		{

			public  ref class ServiceCollectionExtensions
			{
			public:
				static Microsoft::Extensions::DependencyInjection::IServiceCollection^ AddGeometryEngineServices(Microsoft::Extensions::DependencyInjection::IServiceCollection^ services);
				
			};
		}
	}
}

