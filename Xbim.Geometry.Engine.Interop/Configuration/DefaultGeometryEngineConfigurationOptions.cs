using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Configuration;

namespace Xbim.Geometry.Engine.Configuration
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
