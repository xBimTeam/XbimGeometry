using System;
using Microsoft.Win32;
using Microsoft.Extensions.Logging;
using Xbim.Common;
using System.Configuration;

namespace Xbim.Geometry.Engine.Interop
{
    internal class XbimPrerequisitesValidator
    {

        
        const string PrequisitesKey = "SuppressVCRuntimeWarning";

        // Key under HKLM that indicates that VC12 runtime has been installed
        const string Vc12RuntimeRegistryKey = @"SOFTWARE\Microsoft\DevDiv\VC\Servicing\12.0";
        const string Vc12Download = @"https://www.microsoft.com/en-us/download/details.aspx?id=40784";

        internal static void Validate()
        {
            // Check that the VC 12 runtime has been installed before invoking the native Geometry Engine
            // to pre-empt issues resolving the native assembly.
            var key = Registry.LocalMachine.OpenSubKey(Vc12RuntimeRegistryKey);
            if (key == null)
            {
                string message = String.Format("The Visual C++ runtime 'VC12' (VS2013) could not be located. This is required for XBim Geometry generation. Please install this from {0} ",
                        Vc12Download);
                if (SuppressPrequisiteErrors())
                {
                    var logger = ApplicationLogging.CreateLogger<XbimCustomAssemblyResolver>();
                    logger.LogWarning(message);
                }
                else
                {
                    message += String.Format("\n\nTo suppress this exception define an AppSetting with key of '{0}' and value of 'true' in app.config", PrequisitesKey);
                    throw new InvalidOperationException(message);
                }
            }

        }

        private static bool SuppressPrequisiteErrors()
        {
            bool isSuppressed = false;
            var keyValue = ConfigurationManager.AppSettings[PrequisitesKey];

            if (keyValue != null)
            {
                bool.TryParse(keyValue, out isSuppressed);
            }

            return isSuppressed;
        }
    }
}
