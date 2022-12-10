using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;
using System;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Configuration;


namespace Xbim.Geometry.Engine.Interop.Tests.Logging
{


    public static class TestLoggerExtensions
    {
        public static ILoggingBuilder AddTestLogger(
            this ILoggingBuilder builder)
        {
            builder.AddConfiguration();

            builder.Services.TryAddEnumerable(
                ServiceDescriptor.Singleton<ILoggerProvider, TestLoggerProvider>());
            return builder;
        }


    }

    public sealed class TestLoggerProvider : ILoggerProvider
    {
        public ILogger CreateLogger(string categoryName)
        {

            return TestLogger.Create<TestLoggerProvider>();
        }
        public ILogger<T> CreateLogger<T>()
        {

            return TestLogger.Create<T>();
        }
        public void Dispose()
        {

        }
    }
    public static class TestLogger
    {
        public static ILogger<T> Create<T>()
        {
            var logger = new NUnitLogger<T>();
            return logger;
        }

        class NUnitLogger<T> : ILogger<T>, IDisposable
        {
            private readonly Action<string> output = Console.WriteLine;

            public void Dispose()
            {
            }

            public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception exception,
                Func<TState, Exception, string> formatter) => output(formatter(state, exception));

            public bool IsEnabled(LogLevel logLevel) => true;

            public IDisposable BeginScope<TState>(TState state) => this;
        }
    }


}
