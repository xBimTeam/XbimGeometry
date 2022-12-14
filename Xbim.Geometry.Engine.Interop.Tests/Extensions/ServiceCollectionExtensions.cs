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
            //ensure we have loaded the geometry engine
            LoadGeometryEngineServices(services);
            return services;
        }

        private static void LoadGeometryEngineServices(IServiceCollection services)
        {
            var execAssembly = Assembly.GetExecutingAssembly();

            Assembly geomAssembly;
            if (Environment.Is64BitProcess)
            {
                var geom64Dll = Path.Combine(Path.GetDirectoryName(execAssembly.Location), "win10-x64", "xbim.geometry.engine.dll");
                geomAssembly = Assembly.LoadFile(geom64Dll);
            }
            else //assume 32 bit
            {
                var geom32Dll = Path.Combine(Path.GetDirectoryName(execAssembly.Location), "win10-x86", "xbim.geometry.engine.dll");
                geomAssembly = Assembly.LoadFile(geom32Dll);
            }
            var geomServicesCollectionExt = geomAssembly.GetType("Microsoft.Extensions.DependencyInjection.ServiceCollectionExtensions");
            var svcCollExt = Activator.CreateInstance(geomServicesCollectionExt);
            var method = geomServicesCollectionExt.GetMethod("AddGeometryEngineServices");
            method.Invoke(svcCollExt, new object[] { services });
        }
    }
}

