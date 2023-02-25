using Microsoft.Extensions.DependencyInjection;
using Xbim.Geometry.Engine.Extensions;


namespace Xbim.Geometry.Engine.Tests.Extensions
{

    static public class ServiceCollectionExtension
    {
        public static IServiceCollection AddXbimGeometryServices(this IServiceCollection services)
        {
            services.AddGeometryServices();
            return services;
        }

    }
}

