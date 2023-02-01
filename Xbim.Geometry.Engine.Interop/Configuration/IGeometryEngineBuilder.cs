using Microsoft.Extensions.DependencyInjection;

namespace Xbim.Geometry.Engine.Interop.Configuration
{
    public interface IGeometryEngineBuilder
    {
        IServiceCollection Services { get; }
    }
}
