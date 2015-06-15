using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Xbim.Common.Logging;

namespace Xbim.Geometry.Engine.Interop
{
    internal class XbimCustomAssemblyResolver
    {
        internal const string GeomModuleName = "Xbim.Geometry.Engine";
        internal const string XbimModulePrefix = "Xbim.";

        internal static Assembly ResolverHandler(object sender, ResolveEventArgs args)
        {
            return LoadFile(args.Name);
        }

        private static Assembly LoadFile(string moduleName)
        {
            Assembly assembly = Assembly.GetExecutingAssembly(); // in the Interop asm
            var codepath = new Uri(assembly.CodeBase); // code path always points to the deployed DLL

            // Unlike Location codepath is a URI [file:\\c:\wwwroot\etc\WebApp\bin\Xbim.Geometry.Engine.Interop.dll]
            // presumably because it could be Clickonce, Silverlight or run off UNC path
            var appDir = Path.GetDirectoryName(codepath.LocalPath);


            // var app2Dir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            if (appDir == null)
                return null;

            string libraryPath = null;

            if (moduleName.StartsWith(GeomModuleName))
            {
                // Here we detect the type of CPU architecture 
                // at runtime and select the mixed-mode library 
                // from the corresponding directory.
                // This approach assumes that we only have two 
                // versions of the mixed mode assembly, 
                // X86 and X64, it will not work however on 
                // ARM-based applications or any other non X86/X64 
                // platforms
                var relativeDir = String.Format("{0}{1}.dll",
                    GeomModuleName, (IntPtr.Size == 8) ? "64" : "32");

                libraryPath = Path.Combine(appDir, (IntPtr.Size == 8) ? "x64" : "x86", relativeDir);

                //if the engine file doesn't exist it is quite possible that the path is virtual
                //but physical subdirectory might still exist.
                if (!File.Exists(libraryPath))
                    libraryPath = Path.Combine(IntPtr.Size == 8 ? "x64" : "x86", relativeDir);
            }
            else if (moduleName.StartsWith(XbimModulePrefix) && !moduleName.Contains("resources"))
            {
                // If the *32.dll or *64.dll is loaded from a
                // subdirectory (e.g. plugins folder), .net can
                // fail to resolve its dependencies so this is
                // to give it a helping hand
                var splitName = moduleName.Split(',');
                if (splitName.Length >= 1)
                {
                    libraryPath = Path.Combine(appDir, splitName[0] + ".dll");
                }
            }

            if (libraryPath != null)
            {
                LoggerFactory.GetLogger().Debug("Resolved assembly to: " + libraryPath);
                Assembly geomLoaded = Assembly.LoadFile(libraryPath);
                return geomLoaded;
            }
            return null;
        }
    }
}
