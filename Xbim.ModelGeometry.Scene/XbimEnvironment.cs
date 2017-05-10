using System;
using System.Collections.Generic;
using System.Linq;
using Microsoft.Win32;

// ReSharper disable once CheckNamespace
namespace Xbim.ModelGeometry
{
    public static class XbimEnvironment
    {
        /// <summary>
        /// Tests if the requierd VC++ redist platform is installed for the current execution environment.
        /// </summary>
        /// <returns>true if installed</returns>
        public static bool RedistInstalled(bool? overrideUse64Bit = null)
        {
            var use64 = Is64BitProcess();
            if (overrideUse64Bit.HasValue)
                use64 = overrideUse64Bit.Value;

            var substring = use64
                ? "x64"
                : "x86";

            return InstalledRuntimes().Any(x => x.Contains(substring));
        }

        /// <summary>
        /// Suggests the relevant download URL for the required C++ redistributable setup.
        /// </summary>
        /// <returns>a string pointing to the relevant executable</returns>
        public static string RedistDownloadPath()
        {
            // information taken from
            // http://stackoverflow.com/questions/12206314/detect-if-visual-c-redistributable-for-visual-studio-2012-is-installed/34209692#34209692

            return Is64BitProcess()
                ? @"https://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x64.exe"
                : @"https://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x86.exe";
        }
        
        /// <summary>
        /// Tests for vc2013 redist installation through the registry
        /// </summary>
        /// <returns>A list of matching installations</returns>
        public static IEnumerable<string> InstalledRuntimes()
        {
            var t2 = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Default);
            var t3 = t2.OpenSubKey(@"SOFTWARE\Classes\Installer\Dependencies");
            if (t3 == null)
                yield break;

            foreach (var subKeyName in t3.GetSubKeyNames())
            {
                var t4 = t3.OpenSubKey(subKeyName);
                // ReSharper disable once UseNullPropagation
                if (t4 == null)
                    continue;
                var t5 = t4.GetValue(@"DisplayName");
                // ReSharper disable once UseNullPropagation
                if (t5 == null)
                    continue;
                var s = t5 as string;
                if (s == null)
                    continue;
                if (s.Contains(@"Microsoft Visual C++ 2013 Redistributable"))
                {
                    yield return s;
                }
            }
        }

        /// <summary>
        /// The function used by the underlying interop library to determin what C++ assembly to load.
        /// </summary>
        /// <returns>true for 64 bit environments</returns>
        public static bool Is64BitProcess()
        {
            return (IntPtr.Size == 8);
        }
    }
}
