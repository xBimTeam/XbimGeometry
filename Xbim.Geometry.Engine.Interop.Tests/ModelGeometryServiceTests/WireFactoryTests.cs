using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.NetCore.Tests
{
    public class WireFactoryTests
    {
        #region Setup

        static ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        static MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        static IXModelGeometryService _modelSvc = XbimGeometryEngine.CreateModelGeometryService(_dummyModel, loggerFactory);
        #endregion

      
    }
}
