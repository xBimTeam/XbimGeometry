#pragma once
using namespace Microsoft::Extensions::Hosting;
using namespace System::Threading::Tasks;

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
		
			public ref class GeometryServicesHost : IHostedService
			{

			public:
				virtual Task^ StartAsync(System::Threading::CancellationToken cancellationToken)
				{
					return Task::CompletedTask;
				};

				virtual Task^ StopAsync(System::Threading::CancellationToken cancellationToken)
				{
					return Task::CompletedTask;
				};
			};
		}
	}
}

