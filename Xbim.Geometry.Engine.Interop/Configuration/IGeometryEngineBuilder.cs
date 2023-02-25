using Microsoft.Extensions.DependencyInjection;

namespace Xbim.Geometry.Engine.Configuration
{
    public interface IGeometryEngineBuilder
    {
        IServiceCollection Services { get; }
    }
}
