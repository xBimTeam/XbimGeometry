using Microsoft.Extensions.DependencyInjection;
using System;
using System.IO;
using System.Reflection;
using Xbim.Geometry.Engine.Interop;

namespace Xbim.Geometry.Engine.Interop.Tests.Extensions
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

