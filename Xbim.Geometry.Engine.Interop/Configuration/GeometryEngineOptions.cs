using Xbim.Geometry.Abstractions;

namespace Xbim.Geometry.Engine.Interop.Configuration
{
    /// <summary>
    /// Configuration options relating to the Geometry Engine
    /// </summary>
    public class GeometryEngineOptions
    {
        /// <summary>
        /// Creates a new <see cref="GeometryEngineOptions"/>
        /// </summary>
        public GeometryEngineOptions()
        {
            GeometryEngineVersion = XGeometryEngineVersion.V6;
        }

        /// <summary>
        /// The default version of the native Geometry Engine to use
        /// </summary>
        public XGeometryEngineVersion GeometryEngineVersion { get; set; }

        // TODO: We can provide more runtime Geometry Engine parameters here. 
    }
}
