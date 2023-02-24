using Microsoft.Extensions.DependencyInjection;
using Xbim.Common.Configuration;
using Xbim.Geometry.Engine.Interop;
using Xbim.Geometry.Engine.Interop.Extensions;

namespace Xbim.Geometry.Engine.Tests
{
    internal class test
    {
        public test()
        {
            var services = new ServiceCollection();

            XbimServices.Current.ConfigureServices(s => s.AddXbimToolkit()
            .AddGeometryServices(g => g.SetVersion(Abstractions.XGeometryEngineVersion.V6))
            );
        }
    }
}
