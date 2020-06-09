using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc4.Interfaces;
using Xbim.Geometry.Factories;
using System.Threading;

namespace Xbim.Geometry.Services
{
    public class WireService : IXWireService
    {
        private WireFactory factory;
        public WireService(IXLoggingService loggingService, IXModelService modelService)         
        {
            factory = new WireFactory(loggingService, modelService);
        } 

        public IXWire Build(IIfcCurve curve)
        {
            return factory.Build(curve);
        }

        public Task<IXWire> BuildAsync(IIfcCurve curve, CancellationToken cancellationToken)
        {
            return Task.Run(() =>
            {
                return factory.Build(curve);
            }, cancellationToken);
        }

        public Task<IXWire> BuildAsync(IIfcCurve curve)
        {
            return BuildAsync(curve, CancellationToken.None);
        }

        public IXWire Build(IIfcProfileDef def)
        {
            return factory.Build(def);
        }

        public Task<IXWire> BuildAsync(IIfcProfileDef def, CancellationToken cancellationToken)
        {
            return Task.Run(() =>
            {
                return factory.Build(def);
            }, cancellationToken);
        }

        public Task<IXWire> BuildAsync(IIfcProfileDef def)
        {
            return BuildAsync(def, CancellationToken.None);
        }
    }
}
