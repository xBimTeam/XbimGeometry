using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Xbim.Geometry.Engine.Interop
{

    /// <summary>
    /// A class representing the conventions we use for processor specific Geometry Engine library
    /// </summary>
    internal class XbimArchitectureConventions
    {
        public XbimArchitectureConventions()
        {
            if (Is64BitProcess())
            {
                Suffix = "64";
                SubFolder = "x64";
            }
            else
            {
                Suffix = "32";
                SubFolder = "x86";
            }
        }
           
        /// <summary>
        /// The suffix we apply to platform-specific assemblys in the current process architecture
        /// </summary>
        public string Suffix { get; private set; }
        /// <summary>
        /// The default subfolder to look for platform-specific assemblys in the current process architecture
        /// </summary>
        public string SubFolder { get; private set; }

        public static bool Is64BitProcess()
        {
            return (IntPtr.Size == 8);
        }
    }
}
