using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Factories;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Services
{
    public class CurveService : IXCurveService
    {
        private CurveFactory factory;
        public CurveService(IXLoggingService loggingService, IXModelService modelService)
        {
            factory = new CurveFactory(loggingService, modelService);
        }

        public IXCurve Build(IIfcCurve curve)
        {
            return factory.Build(curve);
        }

        public IXCurve BuildDirectrix(IIfcCurve curve, double? startParam, double? endParam)
        {
            return factory.BuildDirectrix(curve, startParam, endParam);
        }
    }
}
