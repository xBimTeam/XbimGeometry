using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using System;
using System.Diagnostics;
using Xbim.Common.Configuration;

namespace Xbim.Geometry.Engine.Interop.Internal
{
    /// <summary>
    /// An Internal Service provider wrapping the <see cref="XbimServices"/> which supports fall back to a minimal services implementation when the consuming application
    /// does not explicitly register the xbim services on <see cref="XbimServices"/>. The fallback services provides a safe baseline but provides no Logging capability.
    /// XBIM INTERNAL USE ONLY. Prefer <see cref="XbimServices"/>
    /// </summary>
    internal class InternalServiceProvider
    {
        /// <summary>
        /// Gets the internal ServiceProvider
        /// </summary>
        public IServiceProvider ServiceProvider { get; }
        private InternalServiceProvider()
        {
            // Set up minimal services required to use Geometry.
            var services = new ServiceCollection();
            // Provide a NullLogger implementation so DI dependencies are satsisfied. We don't have a concrete Logger implementation we can use
            services.AddSingleton<ILoggerFactory, NullLoggerFactory>();
            services.TryAdd(ServiceDescriptor.Singleton(typeof(ILogger<>), typeof(NullLogger<>)));

            services.AddXbimToolkit(opt => opt.AddGeometryServices());
            ServiceProvider = services.BuildServiceProvider();

            var warning = @$"NOTE: The xbim InternalServices are being used. This fallback service provider has no logging support. To see xbim logs logging ensure you provide a LoggerFactory to {typeof(XbimServices).FullName} at startup - or provide an existing ServiceProvider to the XbimServices. e.g.

XbimServices.Current.ConfigureServices(s => s.AddXbimToolkit(c => c.AddLoggerFactory(loggerFactory)));
// or
XbimServices.Current.UseExternalServiceProvider(serviceProvider);";

            Debug.WriteLine(warning);
            Console.Error.WriteLine(warning);
        }
        private static Lazy<InternalServiceProvider> lazySingleton = new Lazy<InternalServiceProvider>(() => new InternalServiceProvider());

        /// <summary>
        /// Gets the Current instance of the <see cref="InternalServiceProvider"/>
        /// </summary>
        public static InternalServiceProvider Current { get; private set; } = lazySingleton.Value;


        /// <summary>
        /// Gets a Service, falling back to the internal service provider when null
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns>A service object of type <typeparamref name="T"/> or null if no such service</returns>
        public static T GetService<T>()
        {
            T service = XbimServices.Current.ServiceProvider.GetService<T>();
            if(service == null)
            {
                // Fallback to our internalProvider
                service = Current.ServiceProvider.GetService<T>();
            }

            return service;
        }

        /// <summary>
        /// Gets a Service, falling back to the internal service provider when null
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns>A service object of type <typeparamref name="T"/> </returns>
        /// <exception cref="InvalidOperationException"></exception>
        public static T GetRequiredService<T>()
        {
            T service = XbimServices.Current.ServiceProvider.GetService<T>();
            if (service == null)
            {
                // Fallback to our internalProvider, using GetRequiredServices
                service = Current.ServiceProvider.GetRequiredService<T>();
            }

            return service;
        }


        /// <summary>
        /// Gets the xbim LoggerFactory
        /// </summary>
        /// <returns></returns>
        public static ILoggerFactory GetLoggerFactory()
        {
            return XbimServices.Current.GetLoggerFactory();
        }
    }
}
