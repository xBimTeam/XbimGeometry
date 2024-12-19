using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;
using Microsoft.Extensions.Options;
using System;
using System.IO;
using System.Reflection;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Geometry.Engine.Interop.Configuration;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Common.Configuration
{
    /// <summary>
    /// ServiceCollection Extensions for xbim Geometry
    /// </summary>
    public static class ServiceCollectionExtensions
    {

        /// <summary>
        /// Adds xbim geometry services to the specified <see cref="IServiceCollection"/>
        /// </summary>
        /// <param name="services"></param>
        /// <returns></returns>
        [Obsolete("Prefer services.AddXbimToolkit(c => c.AddGeometryServices()) instead")]
        public static IServiceCollection AddXbimGeometryServices(this IServiceCollection services)
        {
            return services.AddXbimGeometryServicesInternal(delegate { });
        }

        /// <summary>
        /// Adds xbim geometry services to the specified <see cref="IServiceCollection"/>
        /// </summary>
        /// <param name="services"></param>
        /// <returns></returns>
        internal static IServiceCollection AddXbimGeometryServicesInternal(this IServiceCollection services)
        {
            return services.AddXbimGeometryServicesInternal(delegate { });
        }


        /// <summary>
        /// Adds xbim geometry services to the specified <see cref="IServiceCollection"/>
        /// </summary>
        /// <param name="services"></param>
        /// <param name="configure"></param>
        /// <returns>The <see cref="IServiceCollection"/> so additional calls can be chained</returns>
        [Obsolete("Prefer services.AddXbimToolkit(c => c.AddGeometryServices()) instead")]
        public static IServiceCollection AddXbimGeometryServices(this IServiceCollection services, Action<IGeometryEngineBuilder> configure)
        {
            return AddXbimGeometryServicesInternal(services, configure);
        }

        /// <summary>
        /// Adds xbim geometry services to the specified <see cref="IServiceCollection"/>
        /// </summary>
        /// <param name="services"></param>
        /// <param name="configure"></param>
        /// <returns>The <see cref="IServiceCollection"/> so additional calls can be chained</returns>
        internal static IServiceCollection AddXbimGeometryServicesInternal(this IServiceCollection services, Action<IGeometryEngineBuilder> configure)
        {
            services.AddOptions();

            // We want the same instance of GE regardless of interface used. This 'forwarding factory' is the only approach current M.E.DI
            services.TryAddScoped<XbimGeometryEngine>();
            services.TryAddScoped<IXbimGeometryEngine>(x => x.GetRequiredService<XbimGeometryEngine>());
            services.TryAddScoped<IXbimManagedGeometryEngine>(x => x.GetRequiredService<XbimGeometryEngine>());
            services.AddFactory<IXbimManagedGeometryEngine>();
  
            services.TryAddSingleton<IXbimGeometryServicesFactory, XbimGeometryServicesFactory>();
            services.TryAddSingleton<XbimGeometryEngineFactory>();
            services.AddNativeGeometryServices();
            
            services.TryAddEnumerable(ServiceDescriptor.Singleton((IConfigureOptions<GeometryEngineOptions>)new DefaultGeometryEngineConfigurationOptions(XGeometryEngineVersion.V6)));
            configure(new GeometryEngineBuilder(services));
            return services;
        }

        /// <summary>
        /// Adds the native dependencies to the Service Collection
        /// </summary>
        /// <param name="services"></param>
        /// <returns></returns>
        private static IServiceCollection AddNativeGeometryServices(this IServiceCollection services)
        {
            // We can't invoke the mixed mode/native code directly. We have to go via reflection to invoke:
            // Xbim.Geometry.DependencyInjection.ServiceCollectionExtensions.AddXbimGeometryEngine(services);

            Assembly geometryAssembly = ManagedGeometryAssembly;
            var serviceExtensionsType = geometryAssembly.GetType(XbimArchitectureConventions.ServiceCollectionExtensionsName);

            var serviceCollectionExtensions = Activator.CreateInstance(serviceExtensionsType);
            var method = serviceExtensionsType.GetMethod(XbimArchitectureConventions.AddGeometryEngineServicesName);
            method.Invoke(serviceCollectionExtensions, new object[] { services });

            return services;
        }


        /// <summary>
        /// Shorthand for adding service of <see cref="Func{T, TResult}"/> where TResult is<typeparamref name="TService"/>
        /// </summary>
        /// <typeparam name="TService"></typeparam>
        /// <param name="serviceCollection"></param>
        /// <returns>The <see cref="IServiceCollection"/></returns>
        internal static IServiceCollection AddFactory<TService>(this IServiceCollection serviceCollection)
            where TService : class
         
        {
            serviceCollection
                .TryAddSingleton<Func<TService>>(sp => sp.GetRequiredService<TService>);
            return serviceCollection;
        }
        

        private static Lazy<Assembly> lazyAssembly = new Lazy<Assembly>(() => 
        {
            
            var assembly = Assembly.GetExecutingAssembly();
#if NETFRAMEWORK
            var codepath = new Uri(assembly.CodeBase);
#else
            var location = assembly.Location;
            var codepath = new Uri(String.IsNullOrEmpty(location) ? AppContext.BaseDirectory : location);
#endif
            return LoadDll(codepath.LocalPath);
        });

        private static Assembly ManagedGeometryAssembly { get => lazyAssembly.Value; } 
        
           
        private static Assembly LoadDll(string currentDir)
        {
            var archFolder = XbimArchitectureConventions.Runtime;
            var dllPath = Path.Combine(currentDir, archFolder, XbimArchitectureConventions.ModuleDllName);

            Assembly geomAssembly;
            if (File.Exists(dllPath))
            {
                geomAssembly = Assembly.LoadFile(dllPath);
            }
            else
            {
                // Functions puts bins in a sub-folder under the runtime. e.g. bin/Debug/net48/bin  - while we auto copy GE bins to the typical parent folder
                var parentDir = Path.GetDirectoryName(currentDir);
                var parentDllPath = Path.Combine(parentDir, archFolder, XbimArchitectureConventions.ModuleDllName);
                if (File.Exists(parentDllPath))
                {
                    geomAssembly = Assembly.LoadFile(parentDllPath);
                }
                else
                {
                    throw new Exception($"Unable to locate v6 Geometry Engine in {archFolder}. \n\nLooked in {dllPath} &\n {parentDllPath}");
                }
            }

            return geomAssembly;
        }
    }
}
