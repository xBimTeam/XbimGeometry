using System;

namespace Xbim.Geometry.Engine.Interop
{
    /// <summary> 
    /// A class representing the conventions we use for processor specific Geometry Engine library
    /// </summary> 
    public class XbimArchitectureConventions
    {

        public static string ModuleDllName => ModuleName + ".dll";

        public static string Runtime => Is64BitProcess ? "win10-x64" : "win10-x86";
        ///// <summary>
        ///// The default subfolder to look for platform-specific assemblys in the current process architecture
        ///// </summary>
        //public string SubFolder { get; private set; }

        /// <summary>
        /// name of the dll that that holds the geometry functionality
        /// </summary>
        public static string ModuleName => "Xbim.Geometry.Engine";

        public static bool Is64BitProcess => IntPtr.Size == 8;

        public static string ServiceCollectionExtensionsName => "Microsoft.Extensions.DependencyInjection.ServiceCollectionExtensions";
        public static string AddGeometryEngineServicesName => "AddGeometryEngineServices";
        public static string GeometryConverterFactoryTypeName => "Xbim.Geometry.Factories.GeometryConverterFactory";
    }
}
