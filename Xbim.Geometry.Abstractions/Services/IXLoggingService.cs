using Microsoft.Extensions.Logging;
using System;

namespace Xbim.Geometry.Abstractions
{
    
    public interface IXLoggingService
    {
        IntPtr LogDelegatePtr { get; }
        ILogger Logger { get; }
        void LogCritical(string logMsg);

        void LogError(string logMsg);

        void LogWarning(string logMsg);

        void LogInformation(string logMsg);

        void LogDebug(string logMsg);
    }
}
