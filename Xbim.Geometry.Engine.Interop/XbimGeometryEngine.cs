using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using System;
using System.IO;
using System.Runtime.CompilerServices;
using Xbim.Common;
using Xbim.Common.Configuration;
using Xbim.Common.Exceptions;
using Xbim.Common.Geometry;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Abstractions.Extensions;
using Xbim.Geometry.Engine.Interop.Configuration;
using Xbim.Ifc;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;



namespace Xbim.Geometry.Engine.Interop
{
    /// <summary>
    /// The xbim Geometry Engine. 
    /// </summary>
    /// <remarks>This mananaged class provides an interoperability layer to the underlying native / "mixed-mode"
    /// geometry engine. As of version 6 it supports switching between different implementation of the core geometry engine  
    /// </remarks>
    public class XbimGeometryEngine : IXbimManagedGeometryEngine
    {
        private const string ModelGeometryServiceKey = "ModelGeometryService";
        private IXbimGeometryEngine _engine;
        private readonly IXbimGeometryServicesFactory factory;
        private readonly ILogger _logger;
        private readonly ILoggerFactory _loggerFactory;

        private GeometryEngineOptions _engineOptions;



        private XbimGeometryEngine() { }



        /// <summary>
        /// Creates an instance of <see cref="XbimGeometryEngine"/>. Note a model must be registered using <see cref="XbimGeometryEngine.RegisterModel(IModel)"/> with the engine before 
        /// invoking any geometry functions.
        /// </summary>
        /// <remarks>Alternatively use <see cref="XbimGeometryEngineFactory"/> to build the engine fully configured</remarks>
        /// <param name="servicesFactory"></param>
        /// <param name="loggerFactory"></param>
        public XbimGeometryEngine(IXbimGeometryServicesFactory servicesFactory, ILoggerFactory loggerFactory) :
            this(servicesFactory, loggerFactory, new GeometryEngineOptions())
        {
        }

        /// <summary>
        /// Creates an instance of <see cref="XbimGeometryEngine"/>. Note a model must be registered using <see cref="XbimGeometryEngine.RegisterModel(IModel)"/> with the engine before 
        /// invoking any geometry functions.
        /// </summary>
        /// <remarks>Alternatively use <see cref="XbimGeometryEngineFactory"/> to build the engine fully configured</remarks>
        /// <param name="servicesFactory"></param>
        /// <param name="loggerFactory"></param>
        /// <param name="geometryOptions"></param>
        public XbimGeometryEngine(IXbimGeometryServicesFactory servicesFactory, ILoggerFactory loggerFactory, GeometryEngineOptions geometryOptions) :
            this(servicesFactory, loggerFactory, Options.Create(geometryOptions))
        {
        }

        /// <summary>
        /// Creates an instance of <see cref="XbimGeometryEngine"/>. Note a model must be registered using <see cref="XbimGeometryEngine.RegisterModel(IModel)"/> with the engine before 
        /// invoking any geometry functions.
        /// </summary>
        /// <remarks>Alternatively use <see cref="XbimGeometryEngineFactory"/> to build the engine fully configured</remarks>
        /// <param name="servicesFactory"></param>
        /// <param name="loggerFactory"></param>
        /// <param name="geometryOptions"></param>
        public XbimGeometryEngine(IXbimGeometryServicesFactory servicesFactory, ILoggerFactory loggerFactory, IOptions<GeometryEngineOptions> geometryOptions = null)
        {
            _engineOptions = geometryOptions == null || geometryOptions.Value == null ? new GeometryEngineOptions() : geometryOptions.Value;

            this.factory = servicesFactory ?? throw new ArgumentNullException(nameof(servicesFactory));
            _loggerFactory = loggerFactory ?? XbimServices.Current.GetLoggerFactory();
            _logger = _loggerFactory.CreateLogger<XbimGeometryEngine>();

            _logger.LogDebug("XbimGeometryEngine constructed successfully");
        }

        
        // This a legacy ctor for backward compatibility and use outside of a DI system

        /// <summary>
        /// Creates an instance of <see cref="XbimGeometryEngine"/> and registers the provided model with the Geometry Engine
        /// </summary>
        /// <param name="model"></param>
        /// <param name="loggerFactory"></param>
        /// <param name="options"></param>
        public XbimGeometryEngine(IModel model, ILoggerFactory loggerFactory, GeometryEngineOptions options = null)
        {
            _engineOptions = options ?? new GeometryEngineOptions();
            var services = XbimServices.Current.ServiceProvider;
            this.factory = services.GetRequiredService<IXbimGeometryServicesFactory>();

            _logger = services.GetRequiredService<ILogger<XbimGeometryEngine>>();
            _loggerFactory = loggerFactory ?? services.GetRequiredService<ILoggerFactory>(); ;

            try
            {
                
                RegisterModel(model);
                
                _logger.LogDebug("XbimGeometryEngine constructed successfully");
            }
            catch (Exception e)
            {
                _logger.LogError(0, e, "Failed to construct XbimGeometryEngine");
                throw;
            }
        }

        /// <summary>
        /// Gets and sets the <see cref="GeometryEngineOptions"/>
        /// </summary>
        public GeometryEngineOptions EngineOptions 
        { 
            get { return _engineOptions; } 
            internal set { _engineOptions = value; } 
        } 

        /// <summary>
        /// Associates a new geometry Engine instance and services with the provided <see cref="IModel"/>
        /// </summary>
        /// <param name="model">The model to register</param>
        /// <exception cref="Exception">If an engine cannot be created</exception>
        public void RegisterModel(IModel model)
        {
            _engine = factory.CreateGeometryEngine(_engineOptions.GeometryEngineVersion, model, _loggerFactory);
            _logger.LogTrace("Created Instance of {fullName}", _engine.GetType().FullName);
            if (_engine == null)
            {
                throw new Exception("Failed to create Geometry Engine");
            }
            EnsureModelTagged(model);

        }

        /// <summary>
        /// Unregisters a model with the underlying geometry Engine services
        /// </summary>
        /// <param name="model"></param>
        
        public void UnregisterModel(IModel model)
        {
            IModel underlyingModel = GetModel(model);
            
            // TODO: Should we be releasing resources?
            underlyingModel.RemoveTagValue(ModelGeometryServiceKey);
        }

        private bool EnsureModelTagged(IModel model)
        {
            bool result = true;
            IModel underlyingModel = GetModel(model);
            if (underlyingModel.GetTagValue<IXModelGeometryService>(ModelGeometryServiceKey, out _) == false)
            {
                result = underlyingModel.AddTagValue(ModelGeometryServiceKey, factory.GeometryConverterFactory.GetUnderlyingModelGeometryService(_engine));
            }
            if (!result)
            {
                _logger.LogError("Model.Tag should be null or a Dictionary<string, object>");
                throw new XbimGeometryException("Failed to initialise Model with the Geometry Engine.\n\n IModel.Tag is expected to be null or a Dictionary<string, object>");
            }

            return result;
        }

        private static IModel GetModel(IModel model)
        {
            IModel underlyingModel = model;
            if (model is IfcStore ifcStore) //special case for stores which wrap the internal model
            {
                underlyingModel = ifcStore.Model; ;
            }

            return underlyingModel;
        }



        public IXModelGeometryService ModelService
        {
            get
            {
                if (_engine == null || factory == null)
                    return default;
                return factory.GeometryConverterFactory.GetUnderlyingModelGeometryService(_engine);
            }
        }
        
        protected IXbimGeometryEngine Engine
        {
            get
            {
                if (_engine == null)
                    throw new InvalidOperationException("Models must be registered before invoking the Geometry Engine");
                return _engine;
            }
        }

        public IXbimGeometryObject Create(IIfcGeometricRepresentationItem ifcRepresentation, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcRepresentation))
            {
                return Engine.Create(ifcRepresentation, null, logger);
            }
        }

        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection,
            double angle, XbimGeometryType storageType, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return Engine.CreateShapeGeometry(geometryObject, precision, deflection, angle, storageType, logger);
            }
        }

        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection, double angle, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return Engine.CreateShapeGeometry(geometryObject, precision, deflection, angle, XbimGeometryType.Polyhedron, logger);
            }
        }
        public XbimShapeGeometry CreateShapeGeometry(IXbimGeometryObject geometryObject, double precision, double deflection, ILogger logger /*, angle = 0.5*/)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return Engine.CreateShapeGeometry(geometryObject, precision, deflection, 0.5, XbimGeometryType.Polyhedron, logger);
            }
        }
        /// <summary>
        /// Values for deflection read from config files
        /// </summary>
        /// <param name="oneMillimetre"></param>
        /// <param name="geometryObject"></param>
        /// <param name="precision"></param>
        /// <param name="logger"></param>
        /// <returns></returns>
        public XbimShapeGeometry CreateShapeGeometry(double oneMillimetre, IXbimGeometryObject geometryObject, double precision, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return Engine.CreateShapeGeometry(oneMillimetre, geometryObject, precision, logger);
            }
        }
        public IXbimSolid CreateSolid(IIfcSweptAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcExtrudedAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRevolvedAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSweptDiskSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcBoundingBox ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSurfaceCurveSweptAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcBooleanClippingResult ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcBooleanOperand ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcHalfSpaceSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcPolygonalBoundedHalfSpace ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcBoxedHalfSpace ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcManifoldSolidBrep ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcFacetedBrep ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcFacetedBrepWithVoids ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcClosedShell ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcCsgPrimitive3D ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcCsgSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSphere ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcBlock ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRightCircularCylinder ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRightCircularCone ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRectangularPyramid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSweptDiskSolidPolygonal ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcRevolvedAreaSolidTapered ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcFixedReferenceSweptAreaSolid ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcAdvancedBrep ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcAdvancedBrepWithVoids ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcSectionedSpine ifcSolid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolid(ifcSolid, logger);
            }
        }

        public IXbimShell CreateShell(IIfcOpenShell shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return Engine.CreateShell(shell, logger);
            }
        }

        public IXbimShell CreateShell(IIfcConnectedFaceSet shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return Engine.CreateShell(shell, logger);
            }
        }

        public IXbimShell CreateShell(IIfcSurfaceOfLinearExtrusion linExt, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, linExt))
            {
                return Engine.CreateShell(linExt, logger);
            }
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcTriangulatedFaceSet shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return Engine.CreateSurfaceModel(shell, logger);
            }
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcShellBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return Engine.CreateSurfaceModel(ifcSurface, logger);
            }
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcFaceBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return Engine.CreateSurfaceModel(ifcSurface, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcTriangulatedFaceSet shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return Engine.CreateSolidSet(shell, logger);
            }
        }
        public IXbimSolidSet CreateSolidSet(IIfcPolygonalFaceSet shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return Engine.CreateSolidSet(shell, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcShellBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return Engine.CreateSolidSet(ifcSurface, logger);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcFaceBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return Engine.CreateSolidSet(ifcSurface, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcTriangulatedFaceSet shell, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return Engine.CreateSolid(shell, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcShellBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return Engine.CreateSolid(ifcSurface, logger);
            }
        }

        public IXbimSolid CreateSolid(IIfcFaceBasedSurfaceModel ifcSurface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSurface))
            {
                return Engine.CreateSolid(ifcSurface, logger);
            }
        }

        public IXbimFace CreateFace(IIfcProfileDef profileDef, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, profileDef))
            {
                return Engine.CreateFace(profileDef, logger);
            }
        }


        public IXbimFace CreateFace(IIfcCompositeCurve cCurve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, cCurve))
            {
                return Engine.CreateFace(cCurve, logger);
            }
        }
        public IXbimFace CreateFace(IIfcPolyline pline, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, pline))
            {
                return Engine.CreateFace(pline, logger);
            }
        }

        public IXbimFace CreateFace(IIfcPolyLoop loop, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, loop))
            {
                return Engine.CreateFace(loop, logger);
            }
        }


        public IXbimFace CreateFace(IIfcSurface surface, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, surface))
            {
                return Engine.CreateFace(surface, logger);
            }
        }

        public IXbimFace CreateFace(IIfcPlane plane, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, plane))
            {
                return Engine.CreateFace(plane, logger);
            }
        }

        public IXbimFace CreateFace(IXbimWire wire, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, wire))
            {
                return Engine.CreateFace(wire, logger);
            }
        }

        public IXbimWire CreateWire(IIfcCurve curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateWire(curve, logger);
            }
        }

        public IXbimWire CreateWire(IIfcCompositeCurveSegment compCurveSeg, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, compCurveSeg))
            {
                return Engine.CreateWire(compCurveSeg, logger);
            }
        }



        public IXbimPoint CreatePoint(double x, double y, double z, double tolerance)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger))
            {
                return Engine.CreatePoint(x, y, z, tolerance);
            }
        }

        public IXbimPoint CreatePoint(IIfcCartesianPoint p)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, p))
            {
                return Engine.CreatePoint(p);
            }
        }

        public IXbimPoint CreatePoint(XbimPoint3D p, double tolerance)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, p))
            {
                return Engine.CreatePoint(p, tolerance);
            }
        }

        public IXbimPoint CreatePoint(IIfcPoint pt)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, pt))
            {
                return Engine.CreatePoint(pt);
            }
        }

        public IXbimPoint CreatePoint(IIfcPointOnCurve p, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, p))
            {
                return Engine.CreatePoint(p, logger);
            }
        }

        public IXbimPoint CreatePoint(IIfcPointOnSurface p, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, p))
            {
                return Engine.CreatePoint(p, logger);
            }
        }

        public IXbimVertex CreateVertexPoint(XbimPoint3D point, double precision)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, point))
            {
                return Engine.CreateVertexPoint(point, precision);
            }
        }


        public IXbimSolidSet CreateSolidSet()
        {
            try
            {
                using (new Tracer(LogHelper.CurrentFunctionName(), this._logger))
                {
                    return Engine.CreateSolidSet();
                }
            }
            catch (Exception e)
            {
                _logger.LogError(0, e, "Failed in CreateSolidSet");
                throw new Exception("Engine is not valid", e);
            }

        }

        public IXbimSolidSet CreateSolidSet(IIfcBooleanResult boolOp, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, boolOp))
            {
                return Engine.CreateSolidSet(boolOp, logger);
            }
        }

        public IXbimSolidSet CreateGrid(IIfcGrid grid, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, grid))
            {
                return Engine.CreateGrid(grid, logger);
            }
        }

        public void WriteTriangulation(TextWriter tw, IXbimGeometryObject shape, double tolerance, double deflection)
        {
            WriteTriangulation(tw, shape, tolerance, deflection: deflection, angle: 0.5);
        }

        public void WriteTriangulation(TextWriter tw, IXbimGeometryObject shape, double tolerance, double deflection, double angle)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shape))
            {
                Engine.WriteTriangulation(tw, shape, tolerance, deflection, angle);
            }
        }
        public void WriteTriangulation(BinaryWriter bw, IXbimGeometryObject shape, double tolerance, double deflection, double angle)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shape))
            {
                Engine.WriteTriangulation(bw, shape, tolerance, deflection, angle);
            }
        }

        public void Mesh(IXbimMeshReceiver receiver, IXbimGeometryObject geometryObject, double precision, double deflection,
            double angle = 0.5)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                Engine.Mesh(receiver, geometryObject, precision, deflection, angle);
            }
        }


        public void WriteTriangulation(BinaryWriter bw, IXbimGeometryObject shape, double tolerance, double deflection)
        {
            WriteTriangulation(bw, shape, tolerance, deflection: deflection, angle: 0.5);
        }


        public IXbimGeometryObject Create(IIfcGeometricRepresentationItem ifcRepresentation, IIfcAxis2Placement3D objectLocation, ILogger logger)
        {
            try
            {
                using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcRepresentation))
                {
                    return Engine.Create(ifcRepresentation, objectLocation, logger);
                }
            }
            catch (Exception e)
            {
                (logger ?? _logger).LogError("EE001: Failed to create geometry #{0} of type {1}, {2}", ifcRepresentation.EntityLabel, ifcRepresentation.GetType().Name, e.Message);
                return null;
            }

        }

        public IXbimGeometryObjectSet CreateGeometryObjectSet()
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger))
            {
                return Engine.CreateGeometryObjectSet();
            }
        }

        public IXbimCurve CreateCurve(IIfcCurve curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcPolyline ifcPolyline, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcPolyline))
            {
                return Engine.CreateCurve(ifcPolyline, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcCircle curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcEllipse curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcLine curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcTrimmedCurve curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcBSplineCurveWithKnots curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcRationalBSplineCurveWithKnots curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateCurve(curve, logger);
            }
        }

        public IXbimCurve CreateCurve(IIfcOffsetCurve3D curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateCurve(curve, logger);
            }
        }
        public IXbimCurve CreateCurve(IIfcOffsetCurve2D curve, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, curve))
            {
                return Engine.CreateCurve(curve, logger);
            }
        }

        public XbimMatrix3D ToMatrix3D(IIfcObjectPlacement objPlacement, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, objPlacement))
            {
                return Engine.ToMatrix3D(objPlacement, logger);
            }
        }

        /// <summary>
        /// Transforms an object geomtrically and returns a new object
        /// </summary>
        /// <param name="geometry"></param>
        /// <param name="cartesianTransform"></param>
        /// <returns></returns>
        public IXbimGeometryObject Transformed(IXbimGeometryObject geometry, IIfcCartesianTransformationOperator cartesianTransform)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometry))
            {
                return Engine.Transformed(geometry, cartesianTransform);
            }
        }


        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcAxis2Placement3D placement)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return Engine.Moved(geometryObject, placement);
            }
        }
        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcAxis2Placement2D placement)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return Engine.Moved(geometryObject, placement);
            }
        }
        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcPlacement placement)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return Engine.Moved(geometryObject, placement);
            }
        }

        public IXbimGeometryObject Moved(IXbimGeometryObject geometryObject, IIfcObjectPlacement objectPlacement, ILogger logger)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return Engine.Moved(geometryObject, objectPlacement, logger);
            }
        }

        public IXbimGeometryObject FromBrep(string brepStr)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger))
            {
                return Engine.FromBrep(brepStr);
            }
        }

        public string ToBrep(IXbimGeometryObject geometryObject)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, geometryObject))
            {
                return Engine.ToBrep(geometryObject);
            }
        }

        public IXbimSolidSet CreateSolidSet(IIfcSweptAreaSolid ifcSolid, ILogger logger = null)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, ifcSolid))
            {
                return Engine.CreateSolidSet(ifcSolid, logger);
            }
        }

        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcTessellatedFaceSet shell, ILogger logger = null)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return Engine.CreateSurfaceModel(shell, logger);
            }
        }
        public IXbimGeometryObjectSet CreateSurfaceModel(IIfcPolygonalFaceSet shell, ILogger logger = null)
        {
            using (new Tracer(LogHelper.CurrentFunctionName(), this._logger, shell))
            {
                return Engine.CreateSurfaceModel(shell, logger);
            }
        }

        public void WriteBrep(string filename, IXbimGeometryObject geomObj)
        {
            // no logger is provided so no tracing is started for this function
            Engine.WriteBrep(filename, geomObj);
        }

        public IXbimGeometryObject ReadBrep(string filename)
        {
            // no logger is provided so no tracing is started for this function
            return Engine.ReadBrep(filename);
        }

    }

    public static class LogHelper
    {
        public static string CurrentFunctionName([CallerMemberName] string caller = "")
        {
            return caller;
        }

    }

    /// <summary>
    /// Traces method calls
    /// </summary>
    internal class Tracer : IDisposable
    {
        private readonly string methodName;
        private readonly ILogger logger;

        public Tracer(string methodName, ILogger logger)
        {
            this.methodName = methodName;
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
            logger.LogTrace("Entering GeometryEngine {function}", methodName);
        }

        public Tracer(string methodName, ILogger logger, IPersistEntity entity)
        {
            this.methodName = methodName;
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
            if (logger.IsEnabled(LogLevel.Trace))   // Optimisation to avoid GetType reflection unless Trace enabled
            {
                logger.LogTrace("Entering GeometryEngine {function} with #{entity} [{type}]",
                methodName, entity.EntityLabel, entity.GetType().Name);
            }
        }

        public Tracer(string methodName, ILogger logger, IXbimGeometryObject geometryObject)
        {
            this.methodName = methodName;
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
            if (logger.IsEnabled(LogLevel.Trace)) // Optimisation to avoid GetType reflection unless Trace enabled
            {
                logger.LogTrace("Entering GeometryEngine {function} with {tag} [{type}]",
                    methodName, geometryObject.Tag, geometryObject.GetType().Name);
            }
        }

        public Tracer(string methodName, ILogger logger, XbimPoint3D point)
        {
            this.methodName = methodName;
            this.logger = logger ?? throw new ArgumentNullException(nameof(logger));
            logger.LogTrace("Entering GeometryEngine {function} with point {x},{y},{z}", methodName, point.X, point.Y, point.Z);
        }

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    logger.LogTrace("Exiting GeometryEngine {function}", methodName);
                }

                disposedValue = true;
            }
        }

        ~Tracer()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(false);
        }

        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
            GC.SuppressFinalize(this);  // To avoid excessive GC
        }
        #endregion



    }
}
