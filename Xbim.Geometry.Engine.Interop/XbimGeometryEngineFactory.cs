using Microsoft.Extensions.DependencyInjection;
using System;
using Xbim.Common;
using Xbim.Common.Configuration;
using Xbim.Geometry.Engine.Configuration;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Interop
{
    public class XbimGeometryEngineFactory
    {
        private readonly Func<IXbimManagedGeometryEngine> engineFn;
 
        public XbimGeometryEngineFactory() : 
            this(XbimServices.Current.ServiceProvider.GetRequiredService<Func<IXbimManagedGeometryEngine>>())
        {
            
        }

        /// <summary>
        /// Constructs a <see cref="XbimGeometryEngineFactory"/>
        /// </summary>
        /// <param name="engineFn"></param>
        public XbimGeometryEngineFactory(Func<IXbimManagedGeometryEngine> engineFn)
        {
            this.engineFn = engineFn;
        }

        /// <summary>
        /// Creates a new instance of a <see cref="IXbimGeometryEngine"/> with the provided <see cref="IModel"/> registered with the undelying Engine
        /// </summary>
        /// <param name="model">The model to perform Geometry operations on</param>
        /// <param name="options"><see cref="GeometryEngineOptions"/> (optional)</param>
        /// <returns>A managed <see cref="XbimGeometryEngine"/> instance</returns>
        public IXbimGeometryEngine CreateGeometryEngineForModel(IModel model, GeometryEngineOptions options = default)
        {
            options = options ?? new GeometryEngineOptions();

            var engine = engineFn();
            if(engine is XbimGeometryEngine ge)
            {
                ge.EngineOptions = options;
            }
            engine.RegisterModel(model);
            return engine;
        }
    }
}
