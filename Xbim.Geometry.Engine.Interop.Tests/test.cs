using Microsoft.Extensions.DependencyInjection;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Common;
using Xbim.Common.Configuration;
using Xbim.Geometry.Engine.Interop.Extensions;

namespace Xbim.Geometry.Engine.Interop.Tests
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
