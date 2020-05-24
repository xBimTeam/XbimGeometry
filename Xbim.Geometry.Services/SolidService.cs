using System.Threading;
using System.Threading.Tasks;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Factories;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Services
{
    public class SolidService : IXSolidService
    {
        SolidFactory solidFactory;

        public SolidService(IXLoggingService loggingService, IXModelService modelService)
        {
            solidFactory = new SolidFactory(loggingService, modelService);
        }
        public IXSolid Build(IIfcSolidModel ifcSolid)
        {
            return solidFactory.Build(ifcSolid);
        }

        public IXSolid Build(IIfcCsgPrimitive3D ifcCsgPrimitive)
        {
            return solidFactory.Build(ifcCsgPrimitive);
        }

        public IXSolid Build(IIfcBooleanOperand boolOperand)
        {
            return solidFactory.Build(boolOperand);
        }

        public Task<IXSolid> BuildAsync(IIfcSolidModel ifcSolid)
        {
            return BuildAsync(ifcSolid, CancellationToken.None);
        }

        public Task<IXSolid> BuildAsync(IIfcSolidModel ifcSolid, CancellationToken token)
        {
            return Task.Run(() =>
            {
                return solidFactory.Build(ifcSolid);
            }, token);
        }

        public Task<IXSolid> BuildAsync(IIfcCsgPrimitive3D ifcCsgPrimitive)
        {
            return BuildAsync(ifcCsgPrimitive, CancellationToken.None);
        }

        public Task<IXSolid> BuildAsync(IIfcCsgPrimitive3D ifcCsgPrimitive, CancellationToken token)
        {
            return Task.Run(() =>
            {
                return solidFactory.Build(ifcCsgPrimitive);
            }, token);
        }

        public Task<IXSolid> BuildAsync(IIfcBooleanOperand boolOperand)
        {
            return BuildAsync(boolOperand, CancellationToken.None);
        }

        public Task<IXSolid> BuildAsync(IIfcBooleanOperand boolOperand, CancellationToken token)
        {
            return Task.Run(() =>
            {
                return solidFactory.Build(boolOperand);
            }, token);
        }
    }
}
