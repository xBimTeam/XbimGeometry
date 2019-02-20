#region Directives

using Microsoft.Extensions.Logging;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.Interfaces;
using Xbim.ModelGeometry.Scene.Clustering;
using Xbim.ModelGeometry.Scene.Extensions;
using Xbim.Tessellator;

// ReSharper disable AccessToDisposedClosure

#endregion

namespace Xbim.ModelGeometry.Scene
{
    /// <summary>
    /// Represents a gemetric representation context, i.e. a 'Body' and 'Model'Representation
    /// Note a 3DModelContext may contain multiple IIfcGeometricRepresentationContexts
    /// </summary>
    public class Xbim3DModelContext
    {
        // private static readonly IList<XbimShapeInstance> EmptyShapeList = new List<XbimShapeInstance>(1);

        #region Helper classes

        /// <summary>
        /// Contains all the information needed to perform the meshing of a product with openings and projections
        /// </summary>
        private class XbimProductBooleanInfo
        {
            private readonly int _contextId;
            private readonly int _styleId;
            private readonly int _productLabel;
            private readonly int _productType;
            private IXbimGeometryObjectSet _productGeometries;
            private IXbimSolidSet _cutGeometries;
            private IXbimSolidSet _projectGeometries;
            
            public XbimProductBooleanInfo(XbimCreateContextHelper contextHelper, XbimGeometryEngine engine, IModel model, ConcurrentDictionary<int, bool> shapeIdsUsedMoreThanOnce, IList<XbimShapeInstance> productShapes, IList<XbimShapeInstance> cutToolIds, IList<XbimShapeInstance> projectToolIds, int context, int styleId)
            {
                _contextId = context;
                _styleId = styleId;

                // all the productShapes belong to the same product, take the first to identify it.
                XbimShapeInstance shape = productShapes.FirstOrDefault();
                _productLabel = shape != null ? shape.IfcProductLabel : 0;
                _productType = shape != null ? shape.IfcTypeId : 0;
                AddGeometries(contextHelper, engine, model, shapeIdsUsedMoreThanOnce, productShapes, cutToolIds, projectToolIds);
            }

            private void AddGeometries(XbimCreateContextHelper contextHelper, XbimGeometryEngine engine, IModel model, ConcurrentDictionary<int, bool> shapeIdsUsedMoreThanOnce, IList<XbimShapeInstance> productShapes, IList<XbimShapeInstance> cutToolIds, IList<XbimShapeInstance> projectToolIds)
            {
                // todo: memory: I suppose that the memory footprint grows during meshing because all the shapes are retained in the XbimBooleanDefinition fields
                // this should be investigated and moved.

                bool placebo;
                _productGeometries = engine.CreateGeometryObjectSet();
                foreach (var argument in productShapes)
                {
                    // makes copy if used more than once
                    var geom = contextHelper.GetGeometryFromCache(argument, shapeIdsUsedMoreThanOnce.TryGetValue(argument.ShapeGeometryLabel, out placebo));
                    if (geom != null)
                        _productGeometries.Add(geom);
                    else
                        LogWarning(model.Instances[argument.IfcProductLabel],
                       "Some 3D geometric form definition is missing"
                       );
                }

                //now build the openings
                _cutGeometries = engine.CreateSolidSet();
                foreach (var openingShape in cutToolIds)
                {
                    // makes copy if used more than once
                    bool makeCopy = shapeIdsUsedMoreThanOnce.TryGetValue(openingShape.ShapeGeometryLabel, out placebo);
                    var openingGeom = contextHelper.GetGeometryFromCache(openingShape, makeCopy);
                    if (openingGeom != null)
                        _cutGeometries.Add(openingGeom);
                    else
                        LogWarning(model.Instances[openingShape.IfcProductLabel],
                       "Some 3D geometric form definition is missing");
                }

                //now all the projections
                _projectGeometries = engine.CreateSolidSet();
                foreach (var projectionShape in projectToolIds)
                {
                    var projGeom = contextHelper.GetGeometryFromCache(projectionShape, shapeIdsUsedMoreThanOnce.TryGetValue(projectionShape.ShapeGeometryLabel, out placebo));
                    if (projGeom != null)
                        _projectGeometries.Add(projGeom);
                    else
                        LogWarning(model.Instances[projectionShape.IfcProductLabel],
                       "Some 3D geometric form definition is missing");
                }
            }


            public int ContextId
            {
                get
                {
                    return _contextId;
                }
            }


            public int StyleId
            {
                get
                {
                    return _styleId;
                }
            }

            public IXbimSolidSet CutGeometries
            {
                get { return _cutGeometries; }
            }

            public IXbimSolidSet ProjectGeometries
            {
                get { return _projectGeometries; }
            }

            public IXbimGeometryObjectSet ProductGeometries
            {
                get { return _productGeometries; }
            }

            public int ProductLabel
            {
                get { return _productLabel; }
            }

            public int ProductType
            {
                get { return _productType; }
            }
        }

        //private struct to hold details of references to geometries
        private struct GeometryReference
        {
            public XbimRect3D BoundingBox;
            public int GeometryId;
            public int StyleLabel;
        }

        private class IfcRepresentationContextCollection : KeyedCollection<int, IIfcRepresentationContext>
        {
            protected override int GetKeyForItem(IIfcRepresentationContext item)
            {
                return item.EntityLabel;
            }
        }
        
        private class XbimCreateContextHelper : IDisposable
        {
            internal String InitialiseError;
            internal ConcurrentDictionary<int, GeometryReference> ShapeLookup;
            private bool _disposed;

            public XbimCreateContextHelper(IModel model, IfcRepresentationContextCollection contexts)
            {
                Model = model;
                Contexts = contexts;
            }

            /// <summary>
            /// The key is the IIfc label of the geometry the value is the database record number
            /// </summary>
            internal ConcurrentDictionary<int, int> GeometryShapeLookup { get; private set; }

            private IModel Model { get; set; }
            private IfcRepresentationContextCollection Contexts { get; set; }
            internal ParallelOptions ParallelOptions { get; private set; }
            internal XbimPlacementTree PlacementTree { get; private set; }
            internal HashSet<int> MappedShapeIds { get; private set; }
            internal HashSet<int> FeatureElementShapeIds { get; private set; }
            internal List<IGrouping<IIfcElement, IIfcFeatureElement>> OpeningsAndProjections { get; private set; }
            private HashSet<int> VoidedProductIds { get; set; }
            internal HashSet<int> VoidedShapeIds { get; set; }
            internal HashSet<int> ProductShapeIds { get; private set; }
            internal ConcurrentDictionary<int, IXbimGeometryObject> CachedGeometries { get; private set; }
            internal int Total { get; private set; }
            internal int PercentageParsed { get; set; }
            internal int Tally { get; set; }
            internal Dictionary<int, int> SurfaceStyles { get; private set; }

            internal Dictionary<IIfcRepresentationContext, ConcurrentQueue<XbimBBoxClusterElement>> Clusters
            {
                get;
                private set;
            }

            internal ConcurrentDictionary<int, List<GeometryReference>> MapGeometryReferences { get; private set; }
            internal ConcurrentDictionary<int, XbimMatrix3D> MapTransforms { get; private set; }

            // todo: the custom meshing behaviour needs to be unified.
            // todo: the whole helper needs to be restructured

            public MeshingBehaviourSetter customMeshBehaviour { get; internal set; }

            internal IXbimGeometryObject GetGeometryFromCache(XbimShapeInstance shapeInstance, bool makeCopy)
            {

                int ifcShapeId;
                if (GeometryShapeLookup.TryGetValue(shapeInstance.ShapeGeometryLabel, out ifcShapeId))
                {
                    IXbimGeometryObject obj;
                    if (CachedGeometries.TryGetValue(ifcShapeId, out obj))
                        return makeCopy ? obj.Transform(shapeInstance.Transformation) : obj.TransformShallow(shapeInstance.Transformation);
                } //it might be a map
                else
                {
                    GeometryReference geomRef;
                    if (ShapeLookup.TryGetValue(shapeInstance.InstanceLabel, out geomRef))
                    {
                        IXbimGeometryObject obj;
                        if (CachedGeometries.TryGetValue(geomRef.GeometryId, out obj))
                            return makeCopy ? obj.Transform(shapeInstance.Transformation) : obj.TransformShallow(shapeInstance.Transformation);
                    }
                }
                return null;
            }

            internal bool Initialise(bool adjustWcs)
            {
                try
                {
                    PlacementTree = new XbimPlacementTree(Model, adjustWcs);
                    GeometryShapeLookup = new ConcurrentDictionary<int, int>();
                    MapGeometryReferences = new ConcurrentDictionary<int, List<GeometryReference>>();
                    MapTransforms = new ConcurrentDictionary<int, XbimMatrix3D>();
                    GetOpeningsAndProjections();
                    VoidedProductIds = new HashSet<int>();
                    VoidedShapeIds = new HashSet<int>();
                    ParallelOptions = new ParallelOptions();
                    //ParallelOptions.MaxDegreeOfParallelism = 8;
                    CachedGeometries = new ConcurrentDictionary<int, IXbimGeometryObject>();
                    foreach (var voidedShapeId in OpeningsAndProjections.Select(op => op.Key.EntityLabel))
                        VoidedProductIds.Add(voidedShapeId);
                    GetProductShapeIds();
                    ShapeLookup = new ConcurrentDictionary<int, GeometryReference>();
                    //Get the surface styles
                    GetSurfaceStyles();
                    GetClusters();
                    Total = ProductShapeIds.Count() + OpeningsAndProjections.Count();
                    Tally = 0;
                    PercentageParsed = 0;
                    InitialiseError = "";
                    return true;
                }
                catch (Exception e)
                {
                    InitialiseError = e.Message;
                    return false;
                }
            }

            private void GetClusters()
            {
                Clusters = new Dictionary<IIfcRepresentationContext, ConcurrentQueue<XbimBBoxClusterElement>>();
                foreach (var context in Contexts) Clusters.Add(context, new ConcurrentQueue<XbimBBoxClusterElement>());
            }

            private struct ElementWithFeature
            {
                internal IIfcElement Element;
                internal IIfcFeatureElement Feature;
            }

            private void GetOpeningsAndProjections()
            {

                //srl change the way we handle aggregation as some ifc models now send them in multiple relationships
                //var compoundElementsDictionary =
                //    Model.Instances.OfType<IIfcRelAggregates>()
                //        .Where(x => x.RelatingObject is IIfcElement)
                //        .ToDictionary(x => x.RelatingObject, y => y.RelatedObjects);

                var compoundElementsDictionary = XbimMultiValueDictionary<IIfcObjectDefinition, IIfcObjectDefinition>.Create<HashSet<IIfcObjectDefinition>>();
                foreach (var aggRel in Model.Instances.OfType<IIfcRelAggregates>())
                {
                    foreach (var relObj in aggRel.RelatedObjects)
                    {
                        compoundElementsDictionary.Add(aggRel.RelatingObject, relObj);
                    }

                }

                // openings
                var elementsWithFeatures = new List<ElementWithFeature>();
                var openingRelations = Model.Instances.OfType<IIfcRelVoidsElement>()
                    .Where(
                        r =>
                            r.RelatingBuildingElement != null && r.RelatingBuildingElement.Representation != null &&
                            r.RelatedOpeningElement != null && r.RelatedOpeningElement.Representation != null).ToList();
                foreach (var openingRelation in openingRelations)
                {
                    // process parts
                    ICollection<IIfcObjectDefinition> childrenElements;
                    if (compoundElementsDictionary.TryGetValue(openingRelation.RelatingBuildingElement,
                        out childrenElements))
                    {
                        elementsWithFeatures.AddRange(
                            childrenElements.OfType<IIfcElement>().Select(childElement => new ElementWithFeature()
                            {
                                Element = childElement,
                                Feature = openingRelation.RelatedOpeningElement
                            }));
                    }

                    // process parent
                    elementsWithFeatures.Add(new ElementWithFeature()
                    {
                        Element = openingRelation.RelatingBuildingElement,
                        Feature = openingRelation.RelatedOpeningElement
                    });
                }


                // projections
                var projectingRelations = Model.Instances.OfType<IIfcRelProjectsElement>()
                    .Where(
                        r =>
                            r.RelatingElement.Representation != null &&
                            r.RelatedFeatureElement.Representation != null).ToList();
                foreach (var projectionRelation in projectingRelations)
                {
                    // process parts
                    ICollection<IIfcObjectDefinition> childrenElements;
                    if (compoundElementsDictionary.TryGetValue(projectionRelation.RelatingElement, out childrenElements))
                    {
                        elementsWithFeatures.AddRange(
                            childrenElements.OfType<IIfcElement>().Select(childElement => new ElementWithFeature()
                            {
                                Element = childElement,
                                Feature = projectionRelation.RelatedFeatureElement
                            }));
                    }

                    // process parent
                    elementsWithFeatures.Add(new ElementWithFeature()
                    {
                        Element = projectionRelation.RelatingElement,
                        Feature = projectionRelation.RelatedFeatureElement
                    });
                }

                OpeningsAndProjections = elementsWithFeatures.GroupBy(x => x.Element, y => y.Feature).ToList();
            }


            private void GetSurfaceStyles()
            {
                //get all the surface styles

                var styledItemsGroup = Model.Instances
                    .OfType<IIfcStyledItem>()
                    .Where(s => s.Item != null)
                    .GroupBy(s => s.Item.EntityLabel);
                SurfaceStyles = new Dictionary<int, int>();
                foreach (var styledItemGrouping in styledItemsGroup)
                {
                    var val =
                        styledItemGrouping.SelectMany(st => st.Styles.SelectMany(s => s.SurfaceStyles)).FirstOrDefault();
                    if (val != null)
                    {
                        SurfaceStyles.Add(styledItemGrouping.Key, val.EntityLabel);
                    }
                }
            }


            /// <summary>
            /// populates the  hash sets with the identities of the representation items used in the model
            /// </summary>
            private void GetProductShapeIds()
            {
                MappedShapeIds = new HashSet<int>();
                FeatureElementShapeIds = new HashSet<int>();
                ProductShapeIds = new HashSet<int>();

                foreach (var product in Model.Instances.OfType<IIfcProduct>(true).Where(p => p.Representation != null))
                {

                    if (customMeshBehaviour != null)
                    {
                        double v1 = 0, v2 = 0; // v1 and v2 are ignored in this case
                        var behaviour = customMeshBehaviour(product.EntityLabel, product.ExpressType.TypeId, ref v1, ref v2);
                        if (behaviour == MeshingBehaviourResult.Skip)
                            continue;
                    }

                    var isFeatureElementShape = product is IIfcFeatureElement;
                    var isVoidedProductShape = VoidedProductIds.Contains(product.EntityLabel);
                    //select representations that are in the required context
                    //only want solid representations for this context, but rep type is optional so just filter identified 2d elements
                    //we can only handle one representation in a context and this is in an implementers agreement
                    if (product.Representation != null)
                    {
                        if (product.Representation.Representations == null) continue;
                        var rep =
                            product.Representation.Representations.FirstOrDefault(
                                r => Contexts.Contains(r.ContextOfItems) &&
                                     r.IsBodyRepresentation());
                        //write out the representation if it has one
                        if (rep != null)
                        {
                            foreach (var shape in rep.Items.Where(i => !(i is IIfcGeometricSet)))
                            {
                                var mappedItem = shape as IIfcMappedItem;
                                if (mappedItem != null)
                                {
                                    ProcessMappedItem(isFeatureElementShape, isVoidedProductShape, mappedItem);
                                }
                                else
                                {
                                    //if not already processed add it
                                    ProductShapeIds.Add(shape.EntityLabel);
                                    if (isFeatureElementShape) FeatureElementShapeIds.Add(shape.EntityLabel);
                                    if (isVoidedProductShape) VoidedShapeIds.Add(shape.EntityLabel);
                                }
                            }
                        }
                    }
                }
            }

            private void ProcessMappedItem(bool isFeatureElementShape, bool isVoidedProductShape, IIfcMappedItem mappedItem)
            {
                MappedShapeIds.Add(mappedItem.EntityLabel);
                //make sure any shapes mapped are in the set to process as well
                foreach (var item in mappedItem.MappingSource.MappedRepresentation.Items)
                {
                    if (item is IIfcMappedItem)
                        ProcessMappedItem(isFeatureElementShape, isVoidedProductShape, item as IIfcMappedItem);
                    else if (item != null && !(item is IIfcGeometricSet))
                    {
                        var mappedItemLabel = item.EntityLabel;
                        //if not already processed add it

                        ProductShapeIds.Add(mappedItemLabel);
                        if (isFeatureElementShape) FeatureElementShapeIds.Add(mappedItemLabel);
                        if (isVoidedProductShape) VoidedShapeIds.Add(mappedItemLabel);
                    }
                }
            }
            
            public void Dispose()
            {
                if (_disposed) return;
                _disposed = true;
                if (CachedGeometries != null)
                {
                    foreach (var cachedGeom in CachedGeometries)
                    {
                        if (cachedGeom.Value != null)
                            cachedGeom.Value.Dispose();
                    }
                }
                GC.SuppressFinalize(this);
            }
        }

        #endregion

    
        static private ILogger _logger;
        private readonly IfcRepresentationContextCollection _contexts;
        private XbimGeometryEngine _engine;

        private XbimGeometryEngine Engine
        {
            get { return _engine ?? (_engine = new XbimGeometryEngine()); }
        }

        [Obsolete("Supply ILogger on constructor instead")]
        public static ILogger Logger
        {       
            set
            {
                _logger = value;
            }
        }

        private readonly IModel _model;

        internal static void LogWarning(object entity, string format, params object[] args)
        {
            if (_logger != null)
            {
                var msg = String.Format(format, args);
                var ifcEntity = entity as IPersistEntity;
                if (ifcEntity != null)
                    _logger.LogWarning("GeomScene: #{entityLabel}={entityType} [{message}]", 
                        ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
                else
                    _logger.LogWarning("GeomScene: {entityType} [{message}]", entity.GetType().Name, msg);
            }
        }

        internal static void LogInfo(object entity, string format, params object[] args)
        {
            
            if (_logger != null)
            {
                var msg = String.Format(format, args);
                var ifcEntity = entity as IPersistEntity;
                if (ifcEntity != null)
                    _logger.LogInformation("GeomScene: #{entityLabel}={entityType} [{message}]", 
                        ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
                else
                    _logger.LogInformation("GeomScene: {entityType} [{message}]", entity.GetType().Name, msg);
            }
        }

        internal static void LogError(object entity, string format, params object[] args)
        {
            if (_logger != null)
            {
                var msg = String.Format(format, args);
                var ifcEntity = entity as IPersistEntity;
                if (ifcEntity != null)
                    _logger.LogError("GeomScene: #{entityLabel}={entityType} [{message}]", 
                        ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
                else
                    _logger.LogError("GeomScene: {entityType} [{message}]", entity.GetType().Name, msg);
            }
        }

        internal static void LogError(string msg, Exception ex = null)
        {
            if (_logger != null)
            {
                if (ex == null)
                {
                    _logger.LogError(msg);
                }
                else
                {
                    _logger.LogError(ex, msg);
                }
                
            }
        }

        //The maximum extent for any dimension of any products bouding box 
        //private double _maxXyz;

        /// <summary>
        /// Initialises a model context from the model
        /// </summary>
        /// <param name="model"></param>
        /// <param name="contextType"></param>
        /// <param name="requiredContextIdentifier"></param>
        /// <param name="logger"></param>
        public Xbim3DModelContext(IModel model, string contextType = "model", string requiredContextIdentifier = null,
            ILogger logger = null)
        {
            _model = model;
            _logger = logger ?? XbimLogging.CreateLogger<Xbim3DModelContext>();

            // Get the required context

            // because IfcGeometricRepresentationSubContext is indexed but IIfcGeometricRepresentationContext is not we 
            // build a list starting from subcontexts to speed up the lookup, this is a workaround so that the method
            // is fast on xbimModels that have been already generated (without the index).
            //
            var builtContextList = new List<IIfcGeometricRepresentationContext>();
            builtContextList.AddRange(
                model.Instances.OfType<IIfcGeometricRepresentationSubContext>()
                );

            var parentContexts = builtContextList.OfType<IIfcGeometricRepresentationSubContext>().Select(x => x.ParentContext).Distinct().ToList(); // tolist is needed to prevent collection change.
            builtContextList.AddRange(parentContexts);

            // from this moment we are using the same code we were using before on the prepared builtContextList
            // 
            var contexts = builtContextList.Where(
                        c =>
                            string.Compare(c.ContextType, contextType, true) == 0 ||
                            string.Compare(c.ContextType, "design", true) == 0).ToList();
            //allow for incorrect older models


            if (requiredContextIdentifier != null && contexts.Any())
            //filter on the identifier if defined and we have more than one model context
            {
                var subContexts =
                    contexts.Where(c => c.ContextIdentifier.HasValue
                        && requiredContextIdentifier.ToLower().Contains(c.ContextIdentifier.Value.ToString().ToLower())).ToList();
                if (subContexts.Any())
                    contexts = subContexts;
                //filter to use body if specified, if not just strtick with the generat model context (avoids problems with earlier Revit exports where sub contexts were not given)
            }
            if (!contexts.Any())
            {
                //have a look for older standards
                contexts =
                    model.Instances.OfType<IIfcGeometricRepresentationContext>()
                        .Where(
                            c =>
                                string.Compare(c.ContextType, "design", true) == 0 ||
                                string.Compare(c.ContextType, "model", true) == 0).ToList();
                if (contexts.Any())
                {
                    LogInfo(this,
                        "Unable to find any Geometric Representation contexts with Context Type = {0} and Context Identifier = {1}, using Context Type = 'Design' instead. NB This does not comply with IFC 2x3 or greater, the schema is {2}",
                        contextType, requiredContextIdentifier, string.Join(",", model.Header.FileSchema.Schemas));
                }
                else
                {
                    contexts = model.Instances.OfType<IIfcGeometricRepresentationContext>().ToList();
                    if (contexts.Any())
                    {
                        var ctxtString = contexts.Aggregate("", (current, ctxt) => current + (ctxt.ContextType + " "));
                        if (string.IsNullOrWhiteSpace(ctxtString)) ctxtString = "$";
                        LogInfo(this,
                            "Unable to find any Geometric Representation contexts with Context Type = {0} and Context Identifier = {1}, using  available Context Types '{2}'. NB This does not comply with IFC 2x2 or greater",
                            contextType, requiredContextIdentifier, ctxtString.TrimEnd(' '));
                    }
                    else
                    {
                        LogWarning(this,
                            "Unable to find any Geometric Representation contexts in this file, it is illegal and does not comply with IFC 2x2 or greater");
                    }
                }
            }
            _contexts = new IfcRepresentationContextCollection();
            if (!contexts.Any())
                return;
            foreach (var context in contexts)
            {
                _contexts.Add(context);
            }
        }

        /// <summary>
        /// Lists the context that have been identified by the 3dModelContext initialisation
        /// </summary>
        public IEnumerable<IIfcRepresentationContext> Contexts
        {
            get
            {
                foreach (var context in _contexts)
                {
                    yield return context;
                }
            }
        }

        public IModel Model
        {
            get { return _model; }
        }


        /// <param name="progDelegate"></param>
        /// <param name="adjustWcs"></param>       
        /// <returns></returns>
        public bool CreateContext(ReportProgressDelegate progDelegate = null, bool adjustWcs = true)
        {
            _logger.LogInformation("Starting creation of model scene");
            //NB we no longer support creation of  geometry storage other than binary, other code remains for reading but not writing 
            var geomStorageType = XbimGeometryType.PolyhedronBinary;
            if (_contexts == null || Engine == null)
            {
                _logger.LogWarning("No model context instance or model engine found. Finishing...");
                return false;
            }

            var geometryStore = _model.GeometryStore;

            if (geometryStore == null)
            {
                _logger.LogWarning("No GeometryStore in model. Finishing...");
                return false;
            }

            using (var geometryTransaction = geometryStore.BeginInit())
            {
                if (geometryTransaction == null)
                {
                    _logger.LogWarning("No Transaction created. Finishing...");
                    return false;
                }
                using (var contextHelper = new XbimCreateContextHelper(_model, _contexts))
                {
                    contextHelper.customMeshBehaviour = CustomMeshingBehaviour;
                    if (progDelegate != null) progDelegate(-1, "Initialise");
                    if (!contextHelper.Initialise(adjustWcs))
                        throw new Exception("Failed to initialise geometric context, " + contextHelper.InitialiseError);
                    if (progDelegate != null) progDelegate(101, "Initialise");

                    if (MaxThreads > 0)
                    {
                        contextHelper.ParallelOptions.MaxDegreeOfParallelism = MaxThreads;
                    }

                    WriteShapeGeometries(contextHelper, progDelegate, geometryTransaction, geomStorageType);
                    PrepareMapGeometryReferences(contextHelper, progDelegate);

                    // process features
                    var processed = WriteProductsWithFeatures(contextHelper, progDelegate, geomStorageType, geometryTransaction);

                    if (progDelegate != null) progDelegate(-1, "WriteProductShapes");
                    var productsRemaining = _model.Instances.OfType<IIfcProduct>()
                        .Where(p =>
                            p.Representation != null
                            && !processed.Contains(p.EntityLabel)
                        ).ToList();


                    WriteProductShapes(contextHelper, productsRemaining, geometryTransaction);
                    if (progDelegate != null) progDelegate(101, "WriteProductShapes");
                    //Write out the actual representation item reference count


                    //Write out the regions of the model
                    if (progDelegate != null) progDelegate(-1, "WriteRegionsToDb");
                    foreach (var cluster in contextHelper.Clusters)
                    {
                        WriteRegionsToStore(cluster.Key, cluster.Value, geometryTransaction, contextHelper.PlacementTree.WorldCoordinateSystem);
                    }
                    if (progDelegate != null) progDelegate(101, "WriteRegionsToDb");

                }
                geometryTransaction.Commit();
            }
            _logger.LogInformation("Finished creation of model scene");
            return true;
        }

        [Flags]
        public enum MeshingBehaviourResult
        {
            PerformAdditions = 1,
            PerformSubtractions = 2,
            ReplaceBoundingBox = 4,
            Skip = 8,
            Default = PerformAdditions | PerformSubtractions
        }

        public delegate MeshingBehaviourResult MeshingBehaviourSetter(int elementId, int typeId, ref double linearDeflection,
            ref double angularDeflection);
        
        /// <summary>
        /// A custom function to determine the behaviour and deflection associated with individual items in the mesher.
        /// Default properties can set in the Model.Modelfactors if the same deflection applies to all elements.
        /// </summary>
        public MeshingBehaviourSetter CustomMeshingBehaviour;
        
        /// <summary>
        /// Computes and writes to the DB all shapes of products considering their features (openings and extensions).
        /// The process starts from listing all OpeningsAndProjections (from the context) then performs the solid operations.
        /// </summary>
        private ICollection<int> WriteProductsWithFeatures(XbimCreateContextHelper contextHelper,
            ReportProgressDelegate progDelegate, XbimGeometryType geomType, IGeometryStoreInitialiser txn
            )
        {
            // "processed" has been changed to a concurrentDictonary (it was a hashset) because of multi-threading support in the parrallel loop
            // resulted in fluctuating problems in release mode; the byte value is not used, but is a cheap cost to pay (there's no ConcurrentHashSet implementation in .Net)
            // see http://stackoverflow.com/questions/18922985/concurrent-hashsett-in-net-framework
            var processed = new ConcurrentDictionary<int, byte>();

            var localPercentageParsed = 0;
            var localTally = 0;
            var featureCount = contextHelper.OpeningsAndProjections.Count;
            if (progDelegate != null) progDelegate(-1, "WriteFeatureElements (" + contextHelper.OpeningsAndProjections.Count + " elements)");

            var shapeIdsUsedMoreThanOnce = new ConcurrentDictionary<int, bool>();
            var allShapeIds = new ConcurrentDictionary<int, bool>();

            var openingAndProjectionOps = new ConcurrentBag<XbimProductBooleanInfo>(); // prepares the information to perform the openings and projections
            var precision = Math.Max(_model.ModelFactors.OneMilliMeter / 50, _model.ModelFactors.Precision); //set the precision to 100th mm but never less than precision

            // make sure all the geometries we have cached are sewn
            // contextHelper.SewGeometries(Engine);
            Parallel.ForEach(contextHelper.OpeningsAndProjections, contextHelper.ParallelOptions, elementToFeatureGroup =>
            //         foreach (IGrouping<IIfcElement, IIfcFeatureElement> pair in contextHelper.OpeningsAndProjections)
            // foreach (IGrouping<IIfcElement, IIfcFeatureElement> pair in contextHelper.OpeningsAndProjections)
            {
                int context = 0;
                int styleId = 0; //take the style of any part of the main shape
                var element = elementToFeatureGroup.Key;

                // here is where the feature's geometry are calculated
                //
                var elementShapes = WriteProductShape(contextHelper, element, false, txn);
                var productShapes = new List<XbimShapeInstance>();
                foreach (var elemShape in elementShapes)
                {
                    if (!allShapeIds.TryAdd(elemShape.ShapeGeometryLabel, true))
                        shapeIdsUsedMoreThanOnce.TryAdd(elemShape.ShapeGeometryLabel, true);
                    productShapes.Add(elemShape);
                    context = elemShape.RepresentationContext;
                    if (elemShape.StyleLabel > 0)
                        styleId = elemShape.StyleLabel;
                }

                if (productShapes.Count == 0)
                {
                    processed.TryAdd(element.EntityLabel, 0);
                }
                if (productShapes.Count > 0)
                {
                    var cutTools = new List<XbimShapeInstance>();
                    var projectTools = new List<XbimShapeInstance>();
                    foreach (var feature in elementToFeatureGroup)
                    {
                        // here is where the feature's geometry are calculated
                        //
                        var isCut = feature is IIfcFeatureElementSubtraction;
                        var featureShapes = WriteProductShape(contextHelper, feature, false, txn);
                        foreach (var featureShape in featureShapes)
                        {
                            if (!allShapeIds.TryAdd(featureShape.ShapeGeometryLabel, true))
                                shapeIdsUsedMoreThanOnce.TryAdd(featureShape.ShapeGeometryLabel, true);
                            if (isCut)
                                cutTools.Add(featureShape);
                            else
                                projectTools.Add(featureShape);
                        }
                        processed.TryAdd(feature.EntityLabel, 0);

                    }
                    var boolOp = new XbimProductBooleanInfo(contextHelper, Engine, Model, shapeIdsUsedMoreThanOnce, productShapes, cutTools, projectTools, context, styleId);
                    openingAndProjectionOps.Add(boolOp);
                }
            });
            
            // process all the openings and projections starting with the most operations first
            //
            Parallel.ForEach(openingAndProjectionOps.OrderByDescending(b => b.CutGeometries.Count + b.ProjectGeometries.Count), contextHelper.ParallelOptions, openingAndProjectionOp =>
            {
                Interlocked.Increment(ref localTally);
                var elementLabel = 0;
                try
                {
                    if (progDelegate != null)
                    {
                        var newPercentage = Convert.ToInt32((double)localTally / featureCount * 100.0);
                        if (newPercentage > localPercentageParsed)
                        {
                            Interlocked.Exchange(ref localPercentageParsed, newPercentage);
                            progDelegate(localPercentageParsed, "Building Elements");
                        }
                    }
                    if (!openingAndProjectionOp.ProductGeometries.Any())
                        return;

                    elementLabel = openingAndProjectionOp.ProductLabel;
                    var typeId = openingAndProjectionOp.ProductType;

                    // determine quality and behaviour for specific geometry
                    //
                    var mf = _model.ModelFactors;
                    var thisDeflectionDistance = mf.DeflectionTolerance;
                    var thisDeflectionAngle = mf.DeflectionAngle;
                    var behaviour = MeshingBehaviourResult.Default;

                    if (CustomMeshingBehaviour != null)
                    {
                        behaviour = CustomMeshingBehaviour(elementLabel, typeId, ref thisDeflectionDistance, ref thisDeflectionAngle);
                        if (behaviour == MeshingBehaviourResult.Skip)
                            return; // we are in a parallel loop, this continues to the next
                    }

                    // Get all the parts of this element into a set of solid geometries
                    var elementGeom = openingAndProjectionOp.ProductGeometries;
                    // make the finished shape
                    if (behaviour.HasFlag(MeshingBehaviourResult.PerformAdditions) && openingAndProjectionOp.ProjectGeometries.Any())
                    {
                        var nextGeom = elementGeom.Union(openingAndProjectionOp.ProjectGeometries, precision);
                        if (nextGeom.IsValid)
                        {
                            if (nextGeom.First != null && nextGeom.First.IsValid)
                                elementGeom = nextGeom;
                            else
                                LogWarning(_model.Instances[elementLabel], "Projections are an empty shape");
                        }
                        else
                            LogWarning(_model.Instances[elementLabel], "Joining of projections has failed. Projections have been ignored");
                    }


                    if (behaviour.HasFlag(MeshingBehaviourResult.PerformSubtractions) && openingAndProjectionOp.CutGeometries.Any())
                    {
                        var nextGeom = elementGeom.Cut(openingAndProjectionOp.CutGeometries, precision);
                        if (nextGeom.IsValid)
                        {
                            if (nextGeom.First != null && nextGeom.First.IsValid)
                                elementGeom = nextGeom;
                            else
                                LogWarning(_model.Instances[elementLabel],
                                    "Cutting openings has resulted in an empty shape");
                        }
                        else
                            LogWarning(_model.Instances[elementLabel],
                                "Cutting openings has failed. Openings have been ignored");
                    }

                    // now add to the DB     
                    //
                    foreach (var geom in elementGeom)
                    {
                        XbimShapeGeometry shapeGeometry = new XbimShapeGeometry
                        {
                            IfcShapeLabel = elementLabel,
                            GeometryHash = 0,
                            LOD = XbimLOD.LOD_Unspecified,
                            Format = geomType,
                            BoundingBox = elementGeom.BoundingBox
                        };
                        var memStream = new MemoryStream(0x4000);

                        if (geomType == XbimGeometryType.PolyhedronBinary)
                        {
                            using (var bw = new BinaryWriter(memStream))
                            {
                                Engine.WriteTriangulation(bw, geom, mf.Precision,
                                    thisDeflectionDistance, thisDeflectionAngle);
                            }
                        }
                        else
                        {
                            using (var tw = new StreamWriter(memStream))
                            {
                                Engine.WriteTriangulation(tw, geom, mf.Precision,
                                    thisDeflectionDistance, thisDeflectionAngle);
                            }
                        }
                        ((IXbimShapeGeometryData)shapeGeometry).ShapeData = memStream.ToArray();
                        if (shapeGeometry.ShapeData.Length > 0)
                        {
                            var shapeInstance = new XbimShapeInstance
                            {
                                IfcProductLabel = elementLabel,
                                ShapeGeometryLabel = 0, /* This gets Set to appropriate value a few lines below */
                                StyleLabel = openingAndProjectionOp.StyleId,
                                RepresentationType = XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded,
                                RepresentationContext = openingAndProjectionOp.ContextId,
                                IfcTypeId = (short)typeId,
                                Transformation = XbimMatrix3D.Identity,
                                BoundingBox = elementGeom.BoundingBox
                            };

                            shapeInstance.ShapeGeometryLabel = txn.AddShapeGeometry(shapeGeometry);
                            txn.AddShapeInstance(shapeInstance, shapeInstance.ShapeGeometryLabel);
                        }
                    }
                    processed.TryAdd(elementLabel, 0);
                }
                catch (Exception e)
                {
                    LogWarning(_model.Instances[elementLabel],
                        "Contains openings but  its basic geometry can not be built, {0}", e.Message);
                }
                //if (progDelegate != null) progDelegate(101, "FeatureElement, (#" + element.EntityLabel + " ended)");
            });
            contextHelper.PercentageParsed = localPercentageParsed;
            contextHelper.Tally = localTally;
            if (progDelegate != null) progDelegate(101, "WriteFeatureElements, (" + localTally + " written)");
            return processed.Keys;
        }

        private void WriteProductShapes(XbimCreateContextHelper contextHelper, IEnumerable<IIfcProduct> products, IGeometryStoreInitialiser txn)
        {
            var localTally = contextHelper.Tally;
            var localPercentageParsed = contextHelper.PercentageParsed;

            // write any grids we have converted 
            // grids are written here, they are products (parallel loop  below) 
            // but they are not processed there because their representation is (likely) not body (IsBodyRepresentation())
            //
            foreach (var grid in Model.Instances.OfType<IIfcGrid>())
            {
                GeometryReference instance;
                if (contextHelper.ShapeLookup.TryGetValue(grid.EntityLabel, out instance) && grid.Representation != null && grid.Representation.Representations.Count > 0)
                {
                    XbimMatrix3D placementTransform = XbimPlacementTree.GetTransform(grid, contextHelper.PlacementTree, Engine);
                    // int context = 0;
                    var gRep= grid.Representation?.Representations?.FirstOrDefault();
                    var context = gRep.ContextOfItems;
                    var intContext = (context == null) ? 0 : context.EntityLabel;

                    WriteShapeInstanceToStore(instance.GeometryId, instance.StyleLabel, intContext, grid,
                        placementTransform, instance.BoundingBox,
                        XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded, txn);
                    instance.BoundingBox.Transform(placementTransform);
                }
            }

            Parallel.ForEach(products, contextHelper.ParallelOptions, product =>
            {
                // select representations that are in the required context
                // only want solid representations for this context, but rep type is optional so just filter identified 2d elements
                // we can only handle one representation in a context and this is in an implementers agreement
                if (product.Representation == null)
                    return;
                if (product.Representation.Representations == null)
                    return;

                var rep = product.Representation.Representations.FirstOrDefault(r => IsInContext(r) && r.IsBodyRepresentation());
                //write out the representation if it has one
                if (rep != null)
                {
                    WriteProductShape(contextHelper, product, true, txn);
                }
            }
            );
            contextHelper.Tally = localTally;
            contextHelper.PercentageParsed = localPercentageParsed;
        }

        private bool IsInContext(IIfcRepresentation r)
        {
            if (!_contexts.Any()) return true; //if we have no context take everything
            return _contexts.Contains(r.ContextOfItems);
        }


        /// <summary>
        /// Process the products shape and writes the instances of shape geometries to the Database
        /// </summary>
        /// <param name="contextHelper"></param>
        /// <param name="product">the product to write</param>
        /// <param name="includesOpenings"></param>
        /// <param name="txn"></param>
        /// <returns>IEnumerable of XbimShapeInstance that have been written</returns>
        private IEnumerable<XbimShapeInstance> WriteProductShape(XbimCreateContextHelper contextHelper, IIfcProduct product, 
            bool includesOpenings, IGeometryStoreInitialiser txn)
        {
            if (CustomMeshingBehaviour != null)
            {
                double v1 = 0, v2 = 0; // v1 and v2 are ignored in this case
                var behaviour = CustomMeshingBehaviour(product.EntityLabel, product.ExpressType.TypeId, ref v1, ref v2);
                if (behaviour == MeshingBehaviourResult.Skip)
                    return Enumerable.Empty<XbimShapeInstance>();
            }

            if (product.Representation == null)
                return Enumerable.Empty<XbimShapeInstance>();
            if (product.Representation.Representations == null)
                return Enumerable.Empty<XbimShapeInstance>();

            var rep = product.Representation.Representations.FirstOrDefault(r => IsInContext(r) && r.IsBodyRepresentation());
            if (rep == null)
                return Enumerable.Empty<XbimShapeInstance>();

            // logic to classify feature tagging
            var repType = includesOpenings
                ? XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded
                : XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded;
            if (product is IIfcFeatureElement)
            {
                //  might come in here from direct meshing or from meshing of remaining objects; either way mark as appropriate
                repType = XbimGeometryRepresentationType.OpeningsAndAdditionsOnly;
            }

            // transform setup
            var placementTransform = XbimPlacementTree.GetTransform(product, contextHelper.PlacementTree, Engine);
            
            // process the items
            var shapesInstances = WriteProductShapeRepresentationItems(contextHelper, product, txn, rep, repType, placementTransform, rep.Items);
            return shapesInstances;
        }

        private List<XbimShapeInstance> WriteProductShapeRepresentationItems(XbimCreateContextHelper contextHelper, IIfcProduct product, IGeometryStoreInitialiser txn, IIfcRepresentation rep, XbimGeometryRepresentationType repType, XbimMatrix3D placementTransform, IItemSet<IIfcRepresentationItem> representationItems)
        {
            var shapesInstances = new List<XbimShapeInstance>();
            var contextId = rep.ContextOfItems.EntityLabel;
            foreach (var representationItem in representationItems)
            {
                var theMap = representationItem as IIfcMappedItem;
                if (theMap != null)
                {
                    List<GeometryReference> mapGeomIds;
                    var mapId = representationItem.EntityLabel;

                    // todo: we need to clarify the content of MapGeometryReferences
                    //
                    if (contextHelper.MapGeometryReferences.TryGetValue(mapId, out mapGeomIds))
                    //if we have something to write                           
                    {
                        var mapTransform = contextHelper.MapTransforms[mapId];
                        foreach (var mappedGeometryReference in mapGeomIds)
                        {
                            var trans = XbimMatrix3D.Multiply(mapTransform, placementTransform);

                            shapesInstances.Add(
                                WriteShapeInstanceToStore(mappedGeometryReference.GeometryId, mappedGeometryReference.StyleLabel, contextId,
                                    product,
                                    trans, 
                                    mappedGeometryReference.BoundingBox,
                                    repType, txn)
                                );

                            // do not include opening elements in the clusters (to determine the regions)
                            //
                            if (!(product is IIfcOpeningElement))
                            {
                                //transform the bounds
                                var transformedProductBounds = mappedGeometryReference.BoundingBox.Transform(trans);
                                contextHelper.Clusters[rep.ContextOfItems].Enqueue(
                                    new XbimBBoxClusterElement(mappedGeometryReference.GeometryId,transformedProductBounds)
                                    );
                            }
                        }
                    }
                    else
                    {
                        // maps might not be meshed if they were excluded by the custom meshing behaviour
                        //
                        XbimMatrix3D trs;
                        if (contextHelper.MapTransforms.TryGetValue(mapId, out trs))
                        {
                            var mapTransform = contextHelper.MapTransforms[mapId];
                            var trans = XbimMatrix3D.Multiply(mapTransform, placementTransform);
                            shapesInstances.AddRange(
                                WriteProductShapeRepresentationItems(contextHelper, product, txn, rep, repType, trans, theMap.MappingSource.MappedRepresentation.Items)
                                );
                        }
                    }
                }
                else //it is a direct reference to geometry shape
                {
                    GeometryReference instance;
                    if (contextHelper.ShapeLookup.TryGetValue(representationItem.EntityLabel, out instance))
                    {
                        shapesInstances.Add(
                            WriteShapeInstanceToStore(instance.GeometryId, instance.StyleLabel, contextId, product,
                                placementTransform, instance.BoundingBox /*productBounds*/,
                                repType, txn)
                            );
                        // do not include opening elements in the clusters (to determine the regions)
                        //
                        if (!(product is IIfcOpeningElement))
                        {
                            // transform the bounds
                            var transproductBounds = instance.BoundingBox.Transform(placementTransform);
                            contextHelper.Clusters[rep.ContextOfItems].Enqueue(
                                new XbimBBoxClusterElement(instance.GeometryId,
                                    transproductBounds));
                        }
                    }
                }
            }
            return shapesInstances;
        }

        private void PrepareMapGeometryReferences(XbimCreateContextHelper contextHelper, ReportProgressDelegate progDelegate)
        {
            if (progDelegate != null)
                progDelegate(-1, "WriteMappedItems (" + contextHelper.MappedShapeIds.Count + " items)");
            Parallel.ForEach(contextHelper.MappedShapeIds, contextHelper.ParallelOptions, mapId =>
            {
                var entity = _model.Instances[mapId];
                var map = entity as IIfcMappedItem;
                if (map == null)
                {
                    LogError(_model.Instances[entity.EntityLabel], "Is an illegal entity in maps collection");
                    return;
                }
                var mapShapes = new List<GeometryReference>();
                foreach (var mapShape in map.MappingSource.MappedRepresentation.Items)
                {
                    //Check if we have already written this shape geometry, we should have so throw an exception if not
                    GeometryReference counter;
                    var mapShapeLabel = mapShape.EntityLabel;
                    if (contextHelper.ShapeLookup.TryGetValue(mapShapeLabel, out counter))
                    {
                        int style;
                        // try to get style for map first as that would override lower level style
                        if (GetStyleId(contextHelper, map.EntityLabel, out style))
                            counter.StyleLabel = style;
                        // get actual shape style
                        else if (GetStyleId(contextHelper,mapShapeLabel, out style))
                            counter.StyleLabel = style;
                        mapShapes.Add(counter);
                    }

                    else if (!(mapShape is IIfcGeometricSet) && !(mapShape is IIfcMappedItem)) //ignore non solid geometry sets //it might be a map
                    {
                        LogWarning(_model.Instances[mapShape.EntityLabel], "Failed to find shape in map");
                    }
                }
                if (mapShapes.Any()) //if we have something to write
                {
                    contextHelper.MapGeometryReferences.TryAdd(map.EntityLabel, mapShapes);
                }
                var cartesianTransform = map.MappingTarget.ToMatrix3D();
                var localTransform = map.MappingSource.MappingOrigin.ToMatrix3D();
                contextHelper.MapTransforms.TryAdd(map.EntityLabel,
                    XbimMatrix3D.Multiply(cartesianTransform, localTransform));   
            });
            if (progDelegate != null)
                progDelegate(101, "WriteMappedItems, (" + contextHelper.MappedShapeIds.Count + " written)");
        }

        private bool GetStyleId(XbimCreateContextHelper contextHelper, int shapeId, out int styleId)
        {
            if (contextHelper.SurfaceStyles.TryGetValue(shapeId, out styleId))
                return true;

            var item = Model.Instances[shapeId] as IIfcBooleanResult;
            if (item == null)
            {
                styleId = 0;
                return false;
            }

            var operands = new[] { item.FirstOperand, item.SecondOperand }.Where(o => o != null);
            var stack = new Stack<IIfcBooleanOperand>(operands);
            while (stack.Count != 0)
            {
                var o = stack.Pop();
                // use nested style definition if it is defined
                if (contextHelper.SurfaceStyles.TryGetValue(o.EntityLabel, out styleId))
                {
                    return true;
                }

                // check for nested booleans
                item = o as IIfcBooleanResult;
                if (item == null)
                    continue;

                // dig deeper
                if (item.FirstOperand != null)
                    stack.Push(item.FirstOperand);
                if (item.SecondOperand != null)
                    stack.Push(item.SecondOperand);
            }

            // no nested styling found
            styleId = 0;
            return false;
        }

        /// <summary>
        /// Defines the maximum number of threads to use in parallel operations  any value less then 1 is not used..
        /// </summary>
        public int MaxThreads { get; set; }

        private void WriteShapeGeometries(XbimCreateContextHelper contextHelper, ReportProgressDelegate progDelegate, IGeometryStoreInitialiser geometryStore, XbimGeometryType geomStorageType)
        {
            var localPercentageParsed = contextHelper.PercentageParsed;
            var localTally = contextHelper.Tally;
            // var dedupCount = 0;
            var xbimTessellator = new XbimTessellator(Model, geomStorageType);
            //var geomHash = new ConcurrentDictionary<RepresentationItemGeometricHashKey, int>();

            //var mapLookup = new ConcurrentDictionary<int, int>();
            if (progDelegate != null)
                progDelegate(-1, "WriteShapeGeometries (" + contextHelper.ProductShapeIds.Count + " shapes)");
            var precision = Model.ModelFactors.Precision;
            var deflection = Model.ModelFactors.DeflectionTolerance;
            var deflectionAngle = Model.ModelFactors.DeflectionAngle;
            //if we have any grids turn them in to geometry
            foreach (var grid in Model.Instances.OfType<IIfcGrid>())
            {
                using (var geomModel = Engine.CreateGrid(grid,_logger))
                {
                    if (geomModel != null && geomModel.IsValid)
                    {
                        var shapeGeom = Engine.CreateShapeGeometry(geomModel, precision, deflection, deflectionAngle, geomStorageType,_logger);
                        shapeGeom.IfcShapeLabel = grid.EntityLabel;

                        var refCounter = new GeometryReference
                        {
                            BoundingBox = (shapeGeom).BoundingBox,
                            GeometryId = geometryStore.AddShapeGeometry(shapeGeom)
                        };
                        contextHelper.ShapeLookup.TryAdd(shapeGeom.IfcShapeLabel, refCounter);
                    }
                }
            }
            Parallel.ForEach(contextHelper.ProductShapeIds, contextHelper.ParallelOptions, shapeId =>
            {
                Interlocked.Increment(ref localTally);
                IIfcGeometricRepresentationItem shape;
                try
                {
                    shape = (IIfcGeometricRepresentationItem)Model.Instances[shapeId];
                }
                catch (Exception ex)
                {
                    var errmsg = string.Format("Error getting IIfcGeometricRepresentationItem for EntityLabel #{0}. Geometry Ignored.", shapeId);
                    LogError(errmsg, ex);
                    return;
                }
                if (shape == null)
                {
                    var errmsg = string.Format("IIfcGeometricRepresentationItem for EntityLabel #{0} not found. Geometry Ignored.", shapeId);
                    LogError(errmsg);
                    return;
                }
                var isFeatureElementShape = contextHelper.FeatureElementShapeIds.Contains(shapeId);
                var isVoidedProductShape = contextHelper.VoidedShapeIds.Contains(shapeId);
                {
                    try
                    {
                        // Console.WriteLine(shape.GetType().Name);
                        {
                            XbimShapeGeometry shapeGeom = null;
                            IXbimGeometryObject geomModel = null;
                            if (!isFeatureElementShape && !isVoidedProductShape && xbimTessellator.CanMesh(shape)) // if we can mesh the shape directly just do it
                            {
                                shapeGeom = xbimTessellator.Mesh(shape);
                            }
                            else //we need to create a geometry object
                            {
                                geomModel = Engine.Create(shape, _logger);
                                if (geomModel != null && geomModel.IsValid)
                                {
                                    shapeGeom = Engine.CreateShapeGeometry(geomModel, precision, deflection, deflectionAngle, geomStorageType, _logger);
                                    if (isFeatureElementShape)
                                    {
                                        var geomSet = geomModel as IXbimGeometryObjectSet;
                                        if (geomSet != null)
                                        {
                                            var solidSet = Engine.CreateSolidSet();
                                            solidSet.Add(geomSet);
                                            contextHelper.CachedGeometries.TryAdd(shapeId, solidSet);
                                        }
                                        //we need for boolean operations later, add the polyhedron if the face is planar
                                        else contextHelper.CachedGeometries.TryAdd(shapeId, geomModel);
                                    }
                                    else if (isVoidedProductShape)
                                        contextHelper.CachedGeometries.TryAdd(shapeId, geomModel);
                                }
                            }

                            if (shapeGeom == null || shapeGeom.ShapeData == null || shapeGeom.ShapeData.Length == 0)
                                LogInfo(_model.Instances[shapeId], "Is an empty shape");
                            else
                            {
                                shapeGeom.IfcShapeLabel = shapeId;
                                var reference = new GeometryReference
                                {
                                    BoundingBox = shapeGeom.BoundingBox,
                                    GeometryId = geometryStore.AddShapeGeometry(shapeGeom)
                                };
                                int styleLabel;
                                GetStyleId(contextHelper, shapeGeom.IfcShapeLabel, out styleLabel);
                                reference.StyleLabel = styleLabel;
                                contextHelper.ShapeLookup.TryAdd(shapeGeom.IfcShapeLabel, reference);
                                if (contextHelper.CachedGeometries.ContainsKey(shapeGeom.IfcShapeLabel))
                                {
                                    //keep a record of the IFC label and database record mapping
                                    contextHelper.GeometryShapeLookup.TryAdd(shapeGeom.ShapeLabel, shapeGeom.IfcShapeLabel);
                                }

                                //   shapeGeometries.Add(shapeGeom);
                            }
                            if (geomModel != null && geomModel.IsValid && !isFeatureElementShape && !isVoidedProductShape)
                            {
                                geomModel.Dispose();
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        LogError(shape, "Failed to create shape geometry, {0}", e.Message);
                    }
                }

                if (progDelegate != null)
                {
                    var newPercentage = Convert.ToInt32((double)localTally / contextHelper.Total * 100.0);
                    if (newPercentage > localPercentageParsed)
                    {
                        Interlocked.Exchange(ref localPercentageParsed, newPercentage);
                        progDelegate(localPercentageParsed, "Creating Geometry");
                    }
                }
            }
            );

            contextHelper.PercentageParsed = localPercentageParsed;
            contextHelper.Tally = localTally;

            if (progDelegate != null) progDelegate(101, "WriteShapeGeometries, (" + localTally + " written)");
        }


        private void WriteRegionsToStore(IIfcRepresentationContext context, IEnumerable<XbimBBoxClusterElement> elementsToCluster, IGeometryStoreInitialiser txn, XbimMatrix3D WorldCoordinateSystem)
        {
            //set up a world to partition the model
            var metre = _model.ModelFactors.OneMetre;

            var regions = new XbimRegionCollection();
            // the XbimDBSCAN method adopted for clustering produces clusters of contiguous elements.
            // if the maximum size is a problem they could then be split using other algorithms that divide spaces equally
            //
            var v = XbimDbscan.GetClusters(elementsToCluster, 5 * metre); // .OrderByDescending(x => x.GeometryIds.Count);
            var i = 1;
            regions.AddRange(v.Select(item => new XbimRegion("Region " + i++, item.Bound, item.GeometryIds.Count, WorldCoordinateSystem)));
            regions.ContextLabel = context.EntityLabel;
            txn.AddRegions(regions);
        }

        // todo: is xbimShapeInstance the right place for the context id? should it be in the shape instead? 
        // I undestand there's an implementor agreement on the number of contexts, but we know they are not as strict as in the past.
        //

        /// <summary>
        /// Writes the geometry as a string into the database
        /// </summary>
        /// <returns></returns>
        private XbimShapeInstance WriteShapeInstanceToStore(int shapeLabel, int styleLabel, int ctxtId,
            IIfcProduct product, XbimMatrix3D placementTransform, XbimRect3D bounds,
            XbimGeometryRepresentationType repType, IGeometryStoreInitialiser txn)
        {
            var shapeInstance = new XbimShapeInstance
            {
                IfcProductLabel = product.EntityLabel,
                ShapeGeometryLabel = shapeLabel,
                StyleLabel = styleLabel,
                RepresentationType = repType,
                RepresentationContext = ctxtId,
                IfcTypeId = _model.Metadata.ExpressTypeId(product),
                Transformation = placementTransform,
                BoundingBox = bounds
            };

            try
            {
                txn.AddShapeInstance(shapeInstance, shapeLabel);

            }
            catch (Exception e)
            {
                LogError(_model.Instances[product.EntityLabel], "Failed to create geometry, {0}", e.Message);
            }


            return shapeInstance;
        }


        /// <summary>
        /// Returns an enumerable of all XbimShape Instances in the model in this context, retrieveAllData will ensure that
        /// bounding box and transformation data are also retrieved
        /// </summary>
        /// <returns></returns>
        public IEnumerable<XbimShapeInstance> ShapeInstances()
        {
            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var shapeInstance in reader.ShapeInstances)
                {
                    yield return shapeInstance;
                }
            }
        }

        public IEnumerable<XbimShapeGeometry> ShapeGeometries()
        {

            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var shapeGeometry in reader.ShapeGeometries)
                {
                    yield return shapeGeometry;
                }
            }
        }


        public XbimShapeGeometry ShapeGeometry(int shapeGeometryLabel)
        {
            using (var reader = _model.GeometryStore.BeginRead())
            {
                return reader.ShapeGeometry(shapeGeometryLabel);
            }
        }

        public XbimShapeGeometry ShapeGeometry(XbimShapeInstance shapeInstance)
        {
            return ShapeGeometry(shapeInstance.ShapeGeometryLabel);
        }

        public IEnumerable<XbimRegion> GetRegions()
        {
            var contextIds = _contexts.Select(c => c.EntityLabel).ToList();
            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var regions in reader.ContextRegions)
                {
                    if (contextIds.Contains(regions.ContextLabel))
                    {
                        foreach (var region in regions)
                        {
                            yield return region;
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Get the region with the greates population
        /// </summary>
        /// <returns></returns>
        public XbimRegion GetLargestRegion()
        {
            var regions = new XbimRegionCollection();
            foreach (var region in GetRegions())
            {
                regions.Add(region);
            }
            return regions.MostPopulated();
        }


        /// <summary>
        /// Returns all instances of the specified shape geometry
        /// </summary>
        /// <param name="geometry"></param>
        /// <param name="ignoreFeatures">if true any instances of this geometry that are openings or projects are ignored</param>
        /// <returns></returns>
        public IEnumerable<XbimShapeInstance> ShapeInstancesOf(XbimShapeGeometry geometry, bool ignoreFeatures = false)
        {
            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var context in _contexts)
                {
                    foreach (var shapeInstance in reader.ShapeInstancesOfGeometry(geometry.ShapeLabel))
                    {
                        if (!MatchesShapeRequirements(shapeInstance, context, ignoreFeatures))
                            continue;
                        yield return shapeInstance;
                    }
                }
            }
        }

        private bool MatchesShapeRequirements(IXbimShapeInstanceData shapeInstance, IIfcRepresentationContext context, bool ignoreFeatures)
        {
            return context.EntityLabel == shapeInstance.RepresentationContext && // all must belong to relevant RepresentationContext
                (
                    (typeof(IIfcFeatureElement).IsAssignableFrom(_model.Metadata.GetType(shapeInstance.IfcTypeId)) && // is feature
                        shapeInstance.RepresentationType == (byte)XbimGeometryRepresentationType.OpeningsAndAdditionsOnly) // then look for feature shape
                    ||
                    // normal items
                    (
                        (ignoreFeatures && shapeInstance.RepresentationType == (byte)XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded)
                        ||
                        shapeInstance.RepresentationType == (byte)XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded
                    )
                );
        }

        public IEnumerable<XbimShapeInstance> ShapeInstancesOf(int geometryLabel, bool ignoreFeatures = false)
        {
            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var context in _contexts)
                {
                    foreach (var shapeInstance in reader.ShapeInstancesOfGeometry(geometryLabel))
                    {
                        if (!MatchesShapeRequirements(shapeInstance, context, ignoreFeatures))
                            continue;
                        yield return shapeInstance;
                    }
                }
            }
        }

        /// <summary>
        /// Returns the shape instances of the specified product in this context
        /// </summary>
        /// <param name="product"></param>
        /// <returns></returns>
        public IEnumerable<XbimShapeInstance> ShapeInstancesOf(IIfcProduct product)
        {
            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var context in _contexts)
                {
                    foreach (var shapeInstance in reader.ShapeInstancesOfEntity(product))
                    {
                        if (context.EntityLabel == shapeInstance.RepresentationContext)
                        {
                            yield return shapeInstance;
                        }
                    }
                }
            }
        }
        
        /// <summary>
        /// Returns a triangulated mesh geometry fopr the specified shape
        /// </summary>
        /// <returns></returns>
        public IXbimMeshGeometry3D ShapeGeometryMeshOf(int shapeGeometryLabel)
        {
            var sg = ShapeGeometry(shapeGeometryLabel);
            var mg = new XbimMeshGeometry3D();
            mg.Read(sg.ShapeData);
            return mg;
        }

        /// <summary>
        /// Returns a triangulated mesh geometry fopr the specified shape geometry
        /// </summary>
        /// <returns></returns>
        public IXbimMeshGeometry3D ShapeGeometryMeshOf(XbimShapeGeometry shapeGeometry)
        {
            var mg = new XbimMeshGeometry3D();
            mg.Read(shapeGeometry.ShapeData);
            return mg;
        }

        /// <summary>
        /// Returns a triangulated mesh geometry fopr the specified shape instance, all transformations are applied
        /// </summary>
        /// <returns></returns>
        public IXbimMeshGeometry3D ShapeGeometryMeshOf(XbimShapeInstance shapeInstance)
        {
            var sg = ShapeGeometry(shapeInstance.ShapeGeometryLabel);
            var mg = new XbimMeshGeometry3D();
            mg.Add(sg.ShapeData, shapeInstance.IfcTypeId, shapeInstance.IfcProductLabel,
                shapeInstance.InstanceLabel, shapeInstance.Transformation, (short)_model.UserDefinedId);
            return mg;
        }
    }
}