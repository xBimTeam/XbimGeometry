using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Factories;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Services
{
    public class BooleanService : IXBooleanService
    {
        BooleanFactory booleanFactory;
        public BooleanService(IXLoggingService loggingService, IXModelService modelService)
        {
            booleanFactory = new BooleanFactory(loggingService, modelService);
        }

        public IXShape Build(IIfcBooleanResult boolResult)
        {
            return booleanFactory.Build(boolResult);
        }

        public Task<IXShape> BuildAsync(IIfcBooleanResult boolResult)
        {
            return BuildAsync(boolResult, CancellationToken.None);
        }

        public Task<IXShape> BuildAsync(IIfcBooleanResult boolResult, CancellationToken token)
        {
            return Task.Run(() =>
            {
                return booleanFactory.Build(boolResult);
            }, token);
        }
    }
}
