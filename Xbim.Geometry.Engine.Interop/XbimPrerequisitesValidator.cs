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
