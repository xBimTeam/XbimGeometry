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
        /// The function used by the underlying interop library to determin what C++ assembly to load.
        /// </summary>
        /// <returns>true for 64 bit environments</returns>
        public static bool Is64BitProcess()
        {
            return (IntPtr.Size == 8);
        }
    }
}
