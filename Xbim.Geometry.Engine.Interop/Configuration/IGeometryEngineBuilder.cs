using Microsoft.Extensions.DependencyInjection;

namespace Xbim.Geometry.Engine.Interop.Configuration
{
    /// <summary>
    /// Defines the interface of the xbim Geometry Engine Builder used to configure dependencies
    /// </summary>
    public interface IGeometryEngineBuilder
    {
        /// <summary>
        /// Gets the <see cref="IServiceCollection"/> for use during configuration
        /// </summary>
        IServiceCollection Services { get; }
    }
}
