using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Common;
//using Xbim.Geometry.Services;
using Xbim.Ifc2x3.Interfaces;

namespace Xbim.Geometry.Engine.Interop.Factories
{
   // public class VertexFactory : FactoryBase
    //{
               
        //private VertexService vertexService;
        //public VertexFactory(IModel iModel, ILogger logger)
        //    : base(iModel, logger, typeof(VertexFactory))
        //{
        //    vertexService = new VertexService(logger);
        //}
        //public VertexFactory(ILogger logger)
        //    : base(logger, typeof(VertexFactory))
        //{          
        //    vertexService = new VertexService(logger);
        //}
        //protected override void Dispose(bool disposing)
        //{
        //    if (!disposedValue)
        //    {
        //        if (disposing)
        //        {
        //            vertexService.Dispose();
        //            vertexService = null;
        //        }
        //    }
        //    base.Dispose(disposing);
        //}

        //public XbimVertex Create(double x, double y, double z, double tolerance)
        //{
        //    return vertexService.Build(x, y, z, defaultTolerance);
        //}
        //public XbimVertex Create(double x, double y, double z)
        //{
        //    return Create(x, y, z, defaultTolerance);
        //}

        //public XbimVertex Create(IIfcCartesianPoint cp)
        //{
        //    return Create(cp.Coordinates[0], cp.Coordinates[1], cp.Dim == 3 ? cp.Coordinates[0] : 0);
        //}
   // }
}
