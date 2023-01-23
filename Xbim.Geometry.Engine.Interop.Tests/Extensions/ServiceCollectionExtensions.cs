using Microsoft.Extensions.DependencyInjection;
using System;
using System.IO;
using System.Reflection;

namespace Xbim.Geometry.Engine.Interop.Tests.Extensions
{

    static public class ServiceCollectionExtension
    {
        public static IServiceCollection AddXbimGeometryServices(this IServiceCollection services)
        {
            XbimGeometryEngine.AddGeometryServices(services);
            return services;
        }

    }
}

