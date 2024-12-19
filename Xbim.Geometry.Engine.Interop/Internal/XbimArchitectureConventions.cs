using System;
using System.Runtime.InteropServices;

namespace Xbim.Geometry.Engine.Interop
{
    /// <summary> 
    /// A class representing the conventions we use for processor specific Geometry Engine library
    /// </summary> 
    internal class XbimArchitectureConventions
    {

        public static string ModuleDllName => ModuleName + ".dll";

        /// <summary>
        /// Gets the runtime folder for native binaries for the current processor architecture
        /// </summary>
        public static string Runtime => RuntimeInformation.ProcessArchitecture switch
        {
            Architecture.X86 => "win-x86",
            Architecture.X64 => "win-x64",
            // TODO: Architecture.Arm64 etc
            _ => throw new NotImplementedException(RuntimeInformation.ProcessArchitecture.ToString())
        };


        /// <summary>
        /// name of the dll that that holds the geometry functionality
        /// </summary>
        public static string ModuleName => "Xbim.Geometry.Engine";

        public static string ServiceCollectionExtensionsName => "Xbim.Geometry.DependencyInjection.ServiceCollectionExtensions";
        public static string AddGeometryEngineServicesName => "AddGeometryEngineServices";
    }
}
