﻿using Microsoft.Win32;
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
        static readonly ILogger Logger = LoggerFactory.GetLogger();

        const string GeometryModuleName = "Xbim.Geometry.Engine";
        const string XbimModulePrefix = "Xbim.";
        const string PrequisitesKey = "SuppressVCRuntimeWarning";

        // Key under HKLM that indicates that VC12 runtime has been installed
        const string vc12RuntimeRegistryKey = @"SOFTWARE\Microsoft\DevDiv\VC\Servicing\12.0";
        const string vc12Download = @"https://www.microsoft.com/en-us/download/details.aspx?id=40784";

        internal static Assembly ResolverHandler(object sender, ResolveEventArgs args)
        {
            ValidatePrequisites();
            return ProbeForAssembly(args.Name);
        }

        private static void ValidatePrequisites()
        {
            // Check that the VC 12 runtime has been installed before invoking the native Geometry Engine
            // to pre-empt issues resolving the native assembly.
            var key = Registry.LocalMachine.OpenSubKey(vc12RuntimeRegistryKey);
            if(key == null)
            {
                string message = String.Format("The Visual C++ runtime 'VC12' (VS2013) could not be located. This is required for XBim Geometry generation. Please install this from {0} ",
                        vc12Download);
                if (SuppressPrequisiteErrors())
                {
                    Logger.Warn(message);
                }
                else
                {
                    message += String.Format("\n\nTo suppress this exception define an AppSetting with key of '{0}' and value of 'true' in app.config", PrequisitesKey);
                    throw new InvalidOperationException(message);
                }
            }

        }

        // TODO: This approach will need revisiting should we support other CPU architectures such as ARM in future. 

        // In order to support side-by-side deployment of both 32 bit & 64 bit versions of the native Xbim.Geometry.Engine.dll we 
        // firstly give each architecture DLL a unique suffix (32/64), and support loading of the correct image from a sub folder under the
        // application bin. i.e. bin/x86/ and bin/x64/. This folder deployment strategy avoids BadImageFormatException issues with 
        // ASP.NET (amongst others) where the runtime loads all DLLs in the bin folder at startup.
        // As a result we need to overide the default probing rules to locate the assembly, based whether we are 32-bit or 64-bit process.
        private static Assembly ProbeForAssembly(string moduleName)
        {
            Assembly assembly = Assembly.GetExecutingAssembly(); // The Xbim.Geometry.Engine.Interop assembly
            // code base always points to the deployed DLL, which may be different to the executing Location because of Shadow Copying in the AppDomain (e.g. ASP.NET)
            var codepath = new Uri(assembly.CodeBase); 

            // Unlike Assembly.Location, CodeBase is a URI [file:\\c:\wwwroot\etc\WebApp\bin\Xbim.Geometry.Engine.Interop.dll]
            var appDir = Path.GetDirectoryName(codepath.LocalPath);

            if (appDir == null)
            {
                Logger.WarnFormat("Unable to resolve {0} because the system was not able to acquire base location of running application from {1}.", 
                    moduleName, assembly.FullName);
                return null;
            }
                
            string libraryPath = null;

            if (moduleName.StartsWith(GeometryModuleName))
            {
                // Get conventions used by this process architecture
                var conventions = new ArchitectureConventions();                
                
                // Append the relevant suffix
                var filename = String.Format("{0}{1}.dll", GeometryModuleName, conventions.Suffix);

                // Look in relevant Architecture subfolder off the main application deployment
                libraryPath = Path.Combine(appDir, conventions.SubFolder, filename);

                // Try a relative folder to CWD.
                if (!File.Exists(libraryPath))
                {
                    Logger.DebugFormat("Assembly not found at {0}. Attempting relative path...", libraryPath);
                    libraryPath = Path.Combine(conventions.SubFolder, filename);
                }
            }
            else if (moduleName.StartsWith(XbimModulePrefix) && !moduleName.Contains("resources"))
            {
                // TODO: unclear if this has to do with Geometry Resolving. Suggest this gets moved to a dedicated handler with plugins code.
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

            Assembly loadedAssembly = null;
            if (libraryPath != null)
            {
                loadedAssembly = LoadAssembly(moduleName, libraryPath);
            }
            return loadedAssembly;
        }

        private static Assembly LoadAssembly(string moduleName, string assemblyPath)
        {
            if (!File.Exists(assemblyPath))
            {
                Logger.WarnFormat("Failed to locate assembly '{0}'. Attempted to find at '{1}' but gave up.",
                    moduleName, assemblyPath);
                return null;
            }
            else
            {
                Logger.DebugFormat("Loading assembly from: {0}", assemblyPath);
                // Failures can occur at Load if a dependent assembly cannot be found.
                Assembly geomLoaded = Assembly.LoadFile(assemblyPath);
                return geomLoaded;
            }
        }

        private static bool SuppressPrequisiteErrors()
        {
            bool isSuppressed = false;
            var keyValue = System.Configuration.ConfigurationManager.AppSettings[PrequisitesKey];

            if (keyValue != null)
            {
                bool.TryParse(keyValue, out isSuppressed);
            }

            return isSuppressed;
        }

        /// <summary>
        /// A class representing the conventions we use for processor specific Geometry Engine library
        /// </summary>
        private class ArchitectureConventions
        {
            public ArchitectureConventions()
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
            
            public string Suffix { get; set; }
            public string SubFolder { get; set; }

            private static bool Is64BitProcess()
            {
                return (IntPtr.Size == 8);
            }
        }
    }
}
