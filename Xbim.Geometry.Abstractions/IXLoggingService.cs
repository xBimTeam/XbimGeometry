using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    
    public interface IXLoggingService
    {
        IntPtr LogDelegatePtr { get; }

        void LogCritical(string logMsg);

        void LogError(string logMsg);

        void LogWarning(string logMsg);

        void LogInformation(string logMsg);

        void LogDebug(string logMsg);
    }
}
