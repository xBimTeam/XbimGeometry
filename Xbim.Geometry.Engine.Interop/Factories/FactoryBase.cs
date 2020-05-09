using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common;

namespace Xbim.Geometry.Engine.Interop.Factories
{
    public abstract class FactoryBase : IDisposable
    {

        private IDisposable _loggerScope;

        protected ILogger logger;
        protected double defaultTolerance;
        protected double oneMillimeter;

        #region IDisposable Support
        protected bool disposedValue = false; // To detect redundant calls


        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    // TODO: dispose managed state (managed objects).
                    logger.LogInformation("Factory demolished");
                    _loggerScope.Dispose();

                }

                // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
                // TODO: set large fields to null.

                disposedValue = true;
            }
        }

        // TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
        // ~VertexFactory()
        // {
        //   // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
        //   Dispose(false);
        // }

        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
            // TODO: uncomment the following line if the finalizer is overridden above.
            // GC.SuppressFinalize(this);
        }
        #endregion

        protected FactoryBase(IModel iModel, ILogger logger, Type loggerType)
        {
            _loggerScope = logger.BeginScope(new Dictionary<string, object>
            {
                ["model_name"] = iModel.Header.Name,
                ["creating_application"] = iModel.Header.CreatingApplication,
                ["ifc_version"] = iModel.Header.FileSchema.Schemas.FirstOrDefault()
            });
            defaultTolerance = iModel.ModelFactors.Precision;
            oneMillimeter = iModel.ModelFactors.OneMilliMeter;
            this.logger = logger;
        }
        protected FactoryBase(ILogger logger, Type loggerType)
        {
            _loggerScope = logger.BeginScope(new Dictionary<string, object>
            {
                ["model_name"] = "Undefined",
                ["creating_application"] = "xbim_geometry_engine",
                ["ifc_version"] = "Ifc4"
            });
            defaultTolerance = 1e-5;
            oneMillimeter = 1;
            this.logger = logger;

        }

    }
}