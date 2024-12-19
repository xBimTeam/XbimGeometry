using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using Xbim.Geometry.Abstractions;

namespace Xbim.Geometry.Engine.Interop.Configuration
{
    internal class DefaultGeometryEngineConfigurationOptions : ConfigureOptions<GeometryEngineOptions>
    {
        public DefaultGeometryEngineConfigurationOptions(XGeometryEngineVersion version)
            : base(delegate (GeometryEngineOptions options)
            {
                options.GeometryEngineVersion = version;

            })
        {
        }
    }
}
