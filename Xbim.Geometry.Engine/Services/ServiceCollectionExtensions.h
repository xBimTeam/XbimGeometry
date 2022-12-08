#pragma once
#include "../Factories//GeometryConverterFactory.h"

using namespace Microsoft::Extensions::DependencyInjection;
using namespace Xbim::Geometry::Factories;
 
using namespace Xbim::Geometry::Abstractions;
namespace Microsoft
{
	namespace Extensions
	{
		namespace DependencyInjection
		{

			public  ref class ServiceCollectionExtensions
			{
			public:
				static void Test(System::String^ msg) { System::Console::WriteLine(msg); };
				static Microsoft::Extensions::DependencyInjection::IServiceCollection^ AddGeometryEngineServices(Microsoft::Extensions::DependencyInjection::IServiceCollection^ services);
				
			};
		}
	}
}

