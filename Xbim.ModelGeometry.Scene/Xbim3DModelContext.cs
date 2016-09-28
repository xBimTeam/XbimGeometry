#region Directives

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
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
    ///     Represents a gemetric representation context, i.e. a 'Body' and 'Model'Representation
    ///     Note a 3DModelContext may contain multiple IIfcGeometricRepresentationContexts
    /// </summary>
    public class Xbim3DModelContext
    {
        static private readonly IList<XbimShapeInstance> EmptyShapeList = new List<XbimShapeInstance>(1);
        #region Helper classes

        private class XbimBooleanDefinition
        {

            private readonly int _contextId;
            private readonly int _styleId;
            private readonly int _productLabel;
            private readonly int _productType;
            private IXbimGeometryObjectSet _argumentGeometries;
            private IXbimSolidSet _cutGeometries;
            private IXbimSolidSet _projectGeometries;
            public XbimBooleanDefinition(XbimCreateContextHelper contextHelper, XbimGeometryEngine engine, IModel model, ConcurrentDictionary<int, bool> dupIds, IList<XbimShapeInstance> argumentIds, IList<XbimShapeInstance> cutToolIds, IList<XbimShapeInstance> projectToolIds, int context, int styleId)
            {
                _contextId = context;
                _styleId = styleId;
                XbimShapeInstance shape = argumentIds.FirstOrDefault();
                _productLabel = shape != null ? shape.IfcProductLabel : 0;
                _productType = shape != null ? shape.IfcTypeId : 0;
                AddGeometries(contextHelper, engine, model, dupIds, argumentIds, cutToolIds, projectToolIds);
            }


            private void AddGeometries(XbimCreateContextHelper contextHelper, XbimGeometryEngine engine, IModel model, ConcurrentDictionary<int, bool> dupIds, IList<XbimShapeInstance> argumentIds, IList<XbimShapeInstance> cutToolIds, IList<XbimShapeInstance> projectToolIds)
            {
                bool placebo;
                _argumentGeometries = engine.CreateGeometryObjectSet();
                foreach (var argument in argumentIds)
                {

                    var geom = contextHelper.GetGeometryFromCache(argument, dupIds.TryGetValue(argument.ShapeGeometryLabel, out placebo));
                    if (geom != null)
                        _argumentGeometries.Add(geom);
                    else
                        LogWarning(model.Instances[argument.IfcProductLabel],
                       "Some 3D geometric form definition is missing"
                       );
                }

                //now build the openings
                _cutGeometries = engine.CreateSolidSet();
                foreach (var openingShape in cutToolIds)
                {
                    var openingGeom = contextHelper.GetGeometryFromCache(openingShape, dupIds.TryGetValue(openingShape.ShapeGeometryLabel, out placebo));
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
                    var projGeom = contextHelper.GetGeometryFromCache(projectionShape, dupIds.TryGetValue(projectionShape.ShapeGeometryLabel, out placebo));
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

            public IXbimGeometryObjectSet ArgumentGeometries
            {
                get { return _argumentGeometries; }
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

        /*
                private struct RepresentationItemGeometricHashKey
                {
                    private readonly int _hash;
                    private readonly int _item;
                    private readonly IModel _model;

                    public RepresentationItemGeometricHashKey(IIfcRepresentationItem item)
                    {
                        _item = item.EntityLabel;
                        _model = item.ModelOf;
                        try
                        {
                            _hash = item.GetGeometryHashCode();
                        }
                        catch (XbimGeometryException eg)
                        {
                            Logger.WarnFormat("HashCode error in representation of type {0}, err = {1}", item.GetType().Name,
                                eg.Message);
                            _hash = 0;
                        }
                    }

                    public IIfcRepresentationItem RepresentationItem 
                    {
                        get { return _model.InstancesLocal[_item] as IIfcRepresentationItem; }
                    }

                    public override int GetHashCode()
                    {
                        return _hash;
                    }

           
                    public override bool Equals(object obj)
                    {
                        if (!(obj is RepresentationItemGeometricHashKey)) return false;
                        try
                        {
                   
                            return RepresentationItem.GeometricEquals(((RepresentationItemGeometricHashKey)obj).RepresentationItem);
                        }
                        catch (XbimGeometryException eg)
                        {
                            Logger.WarnFormat("Equality error in representation of type {0}, err = {1}",
                                _item.GetType().Name,
                                eg.Message);
                            return false;
                        }
                    }

                    public override string ToString()
                    {
                        return string.Format("{0} Hash[{1}]", _item, _hash);
                    }
                }
        */

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

            internal ConcurrentDictionary<int, List<GeometryReference>> MapsWritten { get; private set; }
            internal ConcurrentDictionary<int, XbimMatrix3D> MapTransforms { get; private set; }

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

            internal bool Initialise()
            {
                try
                {
                    PlacementTree = new XbimPlacementTree(Model);
                    GeometryShapeLookup = new ConcurrentDictionary<int, int>();
                    MapsWritten = new ConcurrentDictionary<int, List<GeometryReference>>();
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
                var compoundElementsDictionary =
                    Model.Instances.OfType<IIfcRelAggregates>()
                        .Where(x => x.RelatingObject is IIfcElement)
                        .ToDictionary(x => x.RelatingObject, y => y.RelatedObjects);


                // openings
                var elementsWithFeatures = new List<ElementWithFeature>();
                var openingRelations = Model.Instances.OfType<IIfcRelVoidsElement>()
                    .Where(
                        r =>
                            r.RelatingBuildingElement!=null && r.RelatingBuildingElement.Representation != null &&
                            r.RelatedOpeningElement!=null && r.RelatedOpeningElement.Representation != null).ToList();
                foreach (var openingRelation in openingRelations)
                {
                    // process parts
                    IItemSet<IIfcObjectDefinition> childrenElements;
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
                    IItemSet<IIfcObjectDefinition> childrenElements;
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
            ///     populates the  hash sets with the identities of the representation items used in the model
            /// </summary>
            private void GetProductShapeIds()
            {
                MappedShapeIds = new HashSet<int>();
                FeatureElementShapeIds = new HashSet<int>();
                ProductShapeIds = new HashSet<int>();

                foreach (var product in Model.Instances.OfType<IIfcProduct>(true).Where(p => p.Representation != null))
                {
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
                foreach (var cachedGeom in CachedGeometries)
                {
                    if(cachedGeom.Value!=null)
                        cachedGeom.Value.Dispose();
                }
                GC.SuppressFinalize(this);
            }

            internal void SewGeometries(XbimGeometryEngine engine)
            {

                Parallel.ForEach(CachedGeometries, ParallelOptions, geom =>
                {
                    var geomSet = geom.Value as IXbimGeometryObjectSet;
                    if (geomSet != null)
                    {
                        var solidSet = engine.CreateSolidSet();
                        solidSet.Add(geom.Value);
                        CachedGeometries.TryUpdate(geom.Key, solidSet, geom.Value);
                    }
                }
                    );
            }
        }

        #endregion

        private static readonly ILogger Logger = LoggerFactory.GetLogger();
        private readonly IfcRepresentationContextCollection _contexts;
        private XbimGeometryEngine _engine;

        private XbimGeometryEngine Engine
        {
            get { return _engine ?? (_engine = new XbimGeometryEngine()); }
        }
        private readonly IModel _model;

        public static void LogWarning(object entity, string format, params object[] args)
        {
            var msg = String.Format(format, args);
            var ifcEntity = entity as IPersistEntity;
            if (ifcEntity != null)
                Logger.WarnFormat("GeomScene: #{0}={1} [{2}]", ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
            else
                Logger.WarnFormat("GeomScene: {0} [{1}]", entity.GetType().Name, msg);
        }
        public static void LogInfo(object entity, string format, params object[] args)
        {
            var msg = String.Format(format, args);
            var ifcEntity = entity as IPersistEntity;
            if (ifcEntity != null)
                Logger.InfoFormat("GeomScene: #{0}={1} [{2}]", ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
            else
                Logger.InfoFormat("GeomScene: {0} [{1}]", entity.GetType().Name, msg);
        }
        public static void LogError(object entity, string format, params object[] args)
        {
            var msg = String.Format(format, args);
            var ifcEntity = entity as IPersistEntity;
            if (ifcEntity != null)
                Logger.ErrorFormat("GeomScene: #{0}={1} [{2}]", ifcEntity.EntityLabel, ifcEntity.GetType().Name, msg);
            else
                Logger.ErrorFormat("GeomScene: {0} [{1}]", entity.GetType().Name, msg);
        }
        //The maximum extent for any dimension of any products bouding box 
        //private double _maxXyz;

        /// <summary>
        ///     Initialises a model context from the model
        /// </summary>
        /// <param name="model"></param>
        /// <param name="contextType"></param>
        /// <param name="contextIdentifier"></param>
        public Xbim3DModelContext(IModel model, string contextType = "model", string contextIdentifier = null)
        {
            _model = model;

            //Get the required context
            //check for old versions
            var contexts =
                model.Instances.OfType<IIfcGeometricRepresentationContext>()
                    .Where(
                        c =>
                            String.Compare(c.ContextType, contextType, true) == 0 ||
                            String.Compare(c.ContextType, "design", true) == 0).ToList();
            //allow for incorrect older models


            if (contextIdentifier != null && contexts.Any())
            //filter on the identifier if defined and we have more than one model context
            {
                var subContexts =
                    contexts.Where(c => c.ContextIdentifier.HasValue && contextIdentifier.ToLower()
                        .Contains(c.ContextIdentifier.Value.ToString().ToLower())).ToList();
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
                                String.Compare(c.ContextType, "design", true) == 0 ||
                                String.Compare(c.ContextType, "model", true) == 0).ToList();
                if (contexts.Any())
                {
                    LogInfo(this,
                        "Unable to find any Geometric Representation contexts with Context Type = {0} and Context Identifier = {1}, using Context Type = 'Design' instead. NB This does not comply with IFC 2x3 or greater, the schema is {2}",
                        contextType, contextIdentifier, string.Join(",", model.Header.FileSchema.Schemas));
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
                            contextType, contextIdentifier, ctxtString.TrimEnd(' '));
                    }
                    else
                    {
                        LogWarning(this,
                            "Unable to find any Geometric Representation contexts in this file, it is illegal and does not comply with IFC 2x2 or greater");
                    }
                }
            }
            _contexts = new IfcRepresentationContextCollection();
            if (contexts.Any())
            {

                //var roundingPrecisions = new Dictionary<IIfcRepresentationContext, int>();
                //var contextLookup = new Dictionary<short, IIfcRepresentationContext>();
                foreach (var context in contexts)
                {
                    _contexts.Add(context);

                }

            }

        }



        public IModel Model
        {
            get { return _model; }
        }


        /// <summary>
        /// </summary>
        /// <param name="progDelegate"></param>       
        /// <returns></returns>
        public bool CreateContext(ReportProgressDelegate progDelegate = null, bool adjustWcs = true)
        {
            //NB we no longer support creation of  geometry storage other than binary, other code remains for reading but not writing 
            var geomStorageType = XbimGeometryType.PolyhedronBinary;
            if (_contexts == null || Engine == null) return false;


            var geometryStore = _model.GeometryStore;

            if (geometryStore == null) return false;
            using (var geometryTransaction = geometryStore.BeginInit())
            {
                if (geometryTransaction == null) return false;
                using (var contextHelper = new XbimCreateContextHelper(_model, _contexts))
                {


                    if (progDelegate != null) progDelegate(-1, "Initialise");
                    if (!contextHelper.Initialise())
                        throw new Exception("Failed to initialise geometric context, " + contextHelper.InitialiseError);
                    if (progDelegate != null) progDelegate(101, "Initialise");

                    WriteShapeGeometries(contextHelper, progDelegate, geometryTransaction, geomStorageType);

                    WriteMappedItems(contextHelper, progDelegate);

                    //start a new task to process features
                    var processed = WriteFeatureElements(contextHelper, progDelegate, geomStorageType, geometryTransaction);

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
                        WriteRegionsToStore(cluster.Key, cluster.Value, geometryTransaction);
                    }
                    if (progDelegate != null) progDelegate(101, "WriteRegionsToDb");

                }
                geometryTransaction.Commit();
            }
            return true;

        }

        private ICollection<int> WriteFeatureElements(XbimCreateContextHelper contextHelper,
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

            var dupIds = new ConcurrentDictionary<int, bool>();
            var allIds = new ConcurrentDictionary<int, bool>();
            var booleanOps = new ConcurrentBag<XbimBooleanDefinition>();
            double precision = Math.Max(_model.ModelFactors.OneMilliMeter / 50, _model.ModelFactors.Precision); //set the precision to 100th mm but never less than precision
                                                                                                                //make sure all the geometries we have cached are sewn
                                                                                                                //   contextHelper.SewGeometries(Engine);
            Parallel.ForEach(contextHelper.OpeningsAndProjections, contextHelper.ParallelOptions, pair =>
            //         foreach (IGrouping<IIfcElement, IIfcFeatureElement> pair in contextHelper.OpeningsAndProjections)
            // foreach (IGrouping<IIfcElement, IIfcFeatureElement> pair in contextHelper.OpeningsAndProjections)
            {
                int context = 0;
                int styleId = 0; //take the style of any part of the main shape
                var element = pair.Key;


                var elementShapes = WriteProductShape(contextHelper, element, false, txn);
                var arguments = new List<XbimShapeInstance>();
                foreach (var elemShape in elementShapes)
                {
                    if (!allIds.TryAdd(elemShape.ShapeGeometryLabel, true))
                        dupIds.TryAdd(elemShape.ShapeGeometryLabel, true);
                    arguments.Add(elemShape);
                    context = elemShape.RepresentationContext;
                    if (elemShape.StyleLabel > 0) styleId = elemShape.StyleLabel;
                }

                if (arguments.Count == 0)
                {
                    processed.TryAdd(element.EntityLabel, 0);
                }
                if (arguments.Count > 0)
                {
                    var cutTools = new List<XbimShapeInstance>();
                    var projectTools = new List<XbimShapeInstance>();
                    foreach (var feature in pair)
                    {
                        var isCut = feature is IIfcFeatureElementSubtraction;

                        var featureShapes = WriteProductShape(contextHelper, feature, false, txn);
                        foreach (var featureShape in featureShapes)
                        {
                            if (!allIds.TryAdd(featureShape.ShapeGeometryLabel, true))
                                dupIds.TryAdd(featureShape.ShapeGeometryLabel, true);
                            if (isCut)
                                cutTools.Add(featureShape);
                            else
                                projectTools.Add(featureShape);
                        }
                        processed.TryAdd(feature.EntityLabel, 0);

                    }
                    var boolOp = new XbimBooleanDefinition(contextHelper, Engine, Model, dupIds, arguments, cutTools, projectTools, context, styleId);
                    booleanOps.Add(boolOp);
                }

            });
            //process all the large ones first
            Parallel.ForEach(booleanOps.OrderByDescending(b => b.CutGeometries.Count + b.ProjectGeometries.Count), contextHelper.ParallelOptions, bop =>
            //  foreach (var bop in booleanOps.OrderByDescending(b => b.CutGeometries.Count + b.ProjectGeometries.Count))
            {
                //  Console.WriteLine("{0} - {1}", bop.CutGeometries.Count, bop.ArgumentGeometries.Count);
                Interlocked.Increment(ref localTally);
                var elementLabel = 0;
                try
                {
                    if (bop.ArgumentGeometries.Any())
                    {
                        elementLabel = bop.ProductLabel;
                        var typeId = bop.ProductType;
                        //Get all the parts of this element into a set of solid geometries
                        var elementGeom = bop.ArgumentGeometries;
                        //make the finished shape
                        if (bop.ProjectGeometries.Any())
                        {
                            var nextGeom = elementGeom.Union(bop.ProjectGeometries, precision);
                            if (nextGeom.IsValid)
                            {
                                if (nextGeom.First != null && nextGeom.First.IsValid)
                                    elementGeom = nextGeom;
                                else
                                    LogWarning(_model.Instances[elementLabel], "Projections are an empty shape");
                            }
                            else
                                LogWarning(_model.Instances[elementLabel],
                               "Joining of projections has failed. Projections have been ignored");
                        }


                        if (bop.CutGeometries.Any())
                        {
                            //do them in batches of 100
                            //for (int i = 0; i < bop.CutGeometries.Count; i += 50)
                            //{

                            var nextGeom = elementGeom.Cut(bop.CutGeometries, precision);
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
                            // }
                        }

                        ////now add to the DB               
                        IModelFactors mf = _model.ModelFactors;
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
                                    Engine.WriteTriangulation(bw, geom, mf.Precision, mf.DeflectionTolerance,
                                        mf.DeflectionAngle);
                                }
                            }
                            else
                            {
                                using (var tw = new StreamWriter(memStream))
                                {
                                    Engine.WriteTriangulation(tw, geom, mf.Precision, mf.DeflectionTolerance,
                                        mf.DeflectionAngle);
                                }
                            }
                            ((IXbimShapeGeometryData)shapeGeometry).ShapeData = memStream.ToArray();
                            if (shapeGeometry.ShapeData.Length > 0)
                            {
                                var shapeInstance = new XbimShapeInstance
                                {
                                    IfcProductLabel = elementLabel,
                                    ShapeGeometryLabel = 0,
                                    /*Set when geometry written*/
                                    StyleLabel = bop.StyleId,
                                    RepresentationType = XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded,
                                    RepresentationContext = bop.ContextId,
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

                    if (progDelegate != null)
                    {
                        var newPercentage = Convert.ToInt32((double)localTally / featureCount * 100.0);
                        if (newPercentage > localPercentageParsed)
                        {
                            Interlocked.Exchange(ref localPercentageParsed, newPercentage);
                            progDelegate(localPercentageParsed, "Building Elements");
                        }
                    }
                }
                catch (Exception e)
                {
                    LogWarning(_model.Instances[elementLabel],
                        "Contains openings but  its basic geometry can not be built, {0}", e.Message);
                }
                //if (progDelegate != null) progDelegate(101, "FeatureElement, (#" + element.EntityLabel + " ended)");
            }
            );
            contextHelper.PercentageParsed = localPercentageParsed;
            contextHelper.Tally = localTally;
            if (progDelegate != null) progDelegate(101, "WriteFeatureElements, (" + localTally + " written)");
            return processed.Keys;
        }




        private void WriteProductShapes(XbimCreateContextHelper contextHelper, IEnumerable<IIfcProduct> products, IGeometryStoreInitialiser txn)
        {
            var localTally = contextHelper.Tally;
            var localPercentageParsed = contextHelper.PercentageParsed;
            //write any grids we have converted
            foreach (var grid in Model.Instances.OfType<IIfcGrid>())
            {
                GeometryReference instance;
                if (contextHelper.ShapeLookup.TryGetValue(grid.EntityLabel, out instance))
                {
                    XbimMatrix3D placementTransform = XbimMatrix3D.Identity;
                    if (grid.ObjectPlacement is IIfcLocalPlacement)
                        placementTransform = contextHelper.PlacementTree[grid.ObjectPlacement.EntityLabel];
                    else if (grid.ObjectPlacement is IIfcGridPlacement)
                        placementTransform = Engine.ToMatrix3D((IIfcGridPlacement)grid.ObjectPlacement);

                    WriteShapeInstanceToStore(instance.GeometryId, instance.StyleLabel, 0, grid,
                        placementTransform, instance.BoundingBox /*productBounds*/,
                        XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded, txn);
                    instance.BoundingBox /*productBounds*/.Transform(placementTransform);
                    //transform the bounds
                    //contextHelper.Clusters.First().Enqueue(
                    //    new XbimBBoxClusterElement(instance.GeometryId,
                    //        transproductBounds));
                }
            }

             Parallel.ForEach(products, contextHelper.ParallelOptions, product =>
           // foreach (var product in products)
            {
                //select representations that are in the required context
                //only want solid representations for this context, but rep type is optional so just filter identified 2d elements
                //we can only handle one representation in a context and this is in an implementers agreement
                var rep =
                    product.Representation.Representations.FirstOrDefault(r => IsInContext(r) &&
                                                                               r.IsBodyRepresentation());
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
        ///     Process the products shape and writes the instances of shape geometries to the Database
        /// </summary>
        /// <param name="contextHelper"></param>
        /// <param name="element">Element to write</param>
        /// <param name="includesOpenings"></param>
        /// <returns>IEnumerable of XbimShapeInstance that have been written</returns>
        private IEnumerable<XbimShapeInstance> WriteProductShape(XbimCreateContextHelper contextHelper,
            IIfcProduct element, bool includesOpenings, IGeometryStoreInitialiser txn)
        {
            //_maxXyz = _model.ModelFactors.OneMetre * 100; //elements bigger than 100 metres should not be considered in region
            var shapesInstances = new List<XbimShapeInstance>();
            var rep = element.Representation.Representations
                .FirstOrDefault(r => IsInContext(r)
                                     && r.IsBodyRepresentation());
            if (rep == null)
                return Enumerable.Empty<XbimShapeInstance>();

            // logic to classify feature tagging
            var repType = includesOpenings
                ? XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded
                : XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded;
            if (element is IIfcFeatureElement)
            {
                //  might come in here from direct meshing or from meshing of remaining objects; either way mark as appropriate
                repType = XbimGeometryRepresentationType.OpeningsAndAdditionsOnly;
            }

            // other setup
            XbimMatrix3D placementTransform = XbimMatrix3D.Identity;
            if (element.ObjectPlacement is IIfcLocalPlacement)
                placementTransform = contextHelper.PlacementTree[element.ObjectPlacement.EntityLabel];
            else if (element.ObjectPlacement is IIfcGridPlacement)
                placementTransform = Engine.ToMatrix3D((IIfcGridPlacement)element.ObjectPlacement);
            var contextId = rep.ContextOfItems.EntityLabel;

            //write out any shapes it has   

            var topLevelItems = rep.Items;

            ConvertMaps(contextHelper, element, txn, shapesInstances, rep, repType, placementTransform, contextId, topLevelItems);
            return shapesInstances;
        }

        private void ConvertMaps(XbimCreateContextHelper contextHelper, IIfcProduct element, IGeometryStoreInitialiser txn, List<XbimShapeInstance> shapesInstances, IIfcRepresentation rep, XbimGeometryRepresentationType repType, XbimMatrix3D placementTransform, int contextId, IItemSet<IIfcRepresentationItem> topLevelItems)
        {
            foreach (var shape in topLevelItems)
            {
                var theMap = shape as IIfcMappedItem;
                if (theMap != null)
                {

                    List<GeometryReference> mapGeomIds;
                    var mapId = shape.EntityLabel;

                    if (contextHelper.MapsWritten.TryGetValue(mapId, out mapGeomIds))
                    //if we have something to write                           
                    {
                        var mapTransform = contextHelper.MapTransforms[mapId];
                        foreach (var instance in mapGeomIds)
                        {
                            var trans = XbimMatrix3D.Multiply(mapTransform, placementTransform);

                            shapesInstances.Add(
                                WriteShapeInstanceToStore(instance.GeometryId, instance.StyleLabel, contextId,
                                    element,
                                    trans, instance.BoundingBox /*productBounds*/,
                                    repType, txn)
                                );
                            var transproductBounds = instance.BoundingBox /*productBounds*/.Transform(placementTransform);
                            //transform the bounds

                            //if (transproductBounds.SizeX < _maxXyz && transproductBounds.SizeY < _maxXyz && transproductBounds.SizeZ < _maxXyz)
                            contextHelper.Clusters[rep.ContextOfItems].Enqueue(
                            new XbimBBoxClusterElement(instance.GeometryId,
                                transproductBounds));
                        }
                    }
                    else
                    {
                        var mapTransform = contextHelper.MapTransforms[mapId];
                        var trans = XbimMatrix3D.Multiply(mapTransform, placementTransform);
                        ConvertMaps(contextHelper, element, txn, shapesInstances, rep, repType, trans, contextId, theMap.MappingSource.MappedRepresentation.Items);
                    }
                }
                else //it is a direct reference to geometry shape
                {
                    GeometryReference instance;
                    if (contextHelper.ShapeLookup.TryGetValue(shape.EntityLabel, out instance))
                    {
                        shapesInstances.Add(
                            WriteShapeInstanceToStore(instance.GeometryId, instance.StyleLabel, contextId, element,
                                placementTransform, instance.BoundingBox /*productBounds*/,
                                repType, txn)
                            );
                        var transproductBounds = instance.BoundingBox /*productBounds*/.Transform(placementTransform);
                        //transform the bounds
                        contextHelper.Clusters[rep.ContextOfItems].Enqueue(
                            new XbimBBoxClusterElement(instance.GeometryId,
                                transproductBounds));
                    }

                }
            }
        }

        private void WriteMappedItems(XbimCreateContextHelper contextHelper, ReportProgressDelegate progDelegate)
        {
            if (progDelegate != null) progDelegate(-1, "WriteMappedItems (" + contextHelper.MappedShapeIds.Count + " items)");
            Parallel.ForEach(contextHelper.MappedShapeIds, contextHelper.ParallelOptions, mapId =>
            //   foreach (var map in allMaps)
            {
                var entity = _model.Instances[mapId];
                var map = entity as IIfcMappedItem;
                var mapShapes = new List<GeometryReference>();
                if (map != null)
                {
                    foreach (var mapShape in map.MappingSource.MappedRepresentation.Items)
                    {
                        //Check if we have already written this shape geometry, we should have so throw an exception if not
                        GeometryReference counter;

                        var mapShapeLabel = mapShape.EntityLabel;

                        if (contextHelper.ShapeLookup.TryGetValue(mapShapeLabel, out counter))
                        {
                            int style;
                            if (contextHelper.SurfaceStyles.TryGetValue(mapShapeLabel, out style))
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
                        contextHelper.MapsWritten.TryAdd(map.EntityLabel, mapShapes);
                    }
                    var cartesianTransform = map.MappingTarget.ToMatrix3D();
                    var localTransform = map.MappingSource.MappingOrigin.ToMatrix3D();
                    contextHelper.MapTransforms.TryAdd(map.EntityLabel,
                        XbimMatrix3D.Multiply(cartesianTransform, localTransform));
                }
                else
                    LogError(_model.Instances[entity.EntityLabel], "Is an illegal entity in maps collection");
            }
                );
            if (progDelegate != null) progDelegate(101, "WriteMappedItems, (" + contextHelper.MappedShapeIds.Count + " written)");
        }


        private void WriteShapeGeometries(XbimCreateContextHelper contextHelper, ReportProgressDelegate progDelegate, IGeometryStoreInitialiser geometryStore, XbimGeometryType geomStorageType)
        {
            var localPercentageParsed = contextHelper.PercentageParsed;
            var localTally = contextHelper.Tally;
            // var dedupCount = 0;
            var xbimTessellator = new XbimTessellator(Model, geomStorageType);
            //var geomHash = new ConcurrentDictionary<RepresentationItemGeometricHashKey, int>();

            //var mapLookup = new ConcurrentDictionary<int, int>();
            if (progDelegate != null) progDelegate(-1, "WriteShapeGeometries (" + contextHelper.ProductShapeIds.Count + " shapes)");
            var precision = Model.ModelFactors.Precision;
            var deflection = Model.ModelFactors.DeflectionTolerance;
            var deflectionAngle = Model.ModelFactors.DeflectionAngle;
            //if we have any grids turn them in to geometry
            foreach (var grid in Model.Instances.OfType<IIfcGrid>())
            {
                using (var geomModel = Engine.CreateGrid(grid))
                {
                    if (geomModel != null && geomModel.IsValid)
                    {
                        var shapeGeom = Engine.CreateShapeGeometry(geomModel, precision, deflection,
                            deflectionAngle, geomStorageType);
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
            // foreach (var shapeId in contextHelper.ProductShapeIds)
            {
                Interlocked.Increment(ref localTally);
                var shape = (IIfcGeometricRepresentationItem)Model.Instances[shapeId];
                //  var key = new RepresentationItemGeometricHashKey(shape);
                var isFeatureElementShape = contextHelper.FeatureElementShapeIds.Contains(shapeId);
                var isVoidedProductShape = contextHelper.VoidedShapeIds.Contains(shapeId);
                //this can be used to remove duplicate shapes but has a memory overhead as large nimber so objects need to be cached

                //var mappedEntityLabel = geomHash.GetOrAdd(key, shapeId);
                //
                //if (!isFeatureElementShape && mappedEntityLabel != shapeId) //it already exists
                //{
                //    mapLookup.TryAdd(shapeId, mappedEntityLabel);
                //    //Interlocked.Increment(ref dedupCount);
                //}
                //else //we have added a new shape geometry
                //

                //  if (shape.EntityLabel == 43823)
                {
                    // Console.WriteLine(((IIfcFacetedBrep) shape).Outer.CfsFaces.Count);
                    try
                    {
                        // Console.WriteLine(shape.GetType().Name);
                        // using (var geomModel = _engine.Create(shape))
                        {
                            XbimShapeGeometry shapeGeom = null;
                            IXbimGeometryObject geomModel = null;
                            if (!isFeatureElementShape && !isVoidedProductShape && xbimTessellator.CanMesh(shape)) //if we can mesh the shape directly just do it
                            {
                                shapeGeom = xbimTessellator.Mesh(shape);
                            }
                            else //we need to create a geometry object
                            {

                                geomModel = Engine.Create(shape);
                                if (geomModel != null && geomModel.IsValid)
                                {
                                    shapeGeom = Engine.CreateShapeGeometry(geomModel, precision, deflection,
                                        deflectionAngle, geomStorageType);
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
                                //remove use of blocking collection
                                var refCounter = new GeometryReference
                                {
                                    BoundingBox = (shapeGeom).BoundingBox,
                                    GeometryId = geometryStore.AddShapeGeometry(shapeGeom)

                                };
                                int styleLabel;
                                contextHelper.SurfaceStyles.TryGetValue(shapeGeom.IfcShapeLabel, out styleLabel);
                                refCounter.StyleLabel = styleLabel;
                                contextHelper.ShapeLookup.TryAdd(shapeGeom.IfcShapeLabel, refCounter);
                                if (contextHelper.CachedGeometries.ContainsKey(shapeGeom.IfcShapeLabel))
                                    //keep a record of the IFC label and database record mapping
                                    contextHelper.GeometryShapeLookup.TryAdd(shapeGeom.ShapeLabel,
                                        shapeGeom.IfcShapeLabel);


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


            //Now tidy up the maps
            //Parallel.ForEach(mapLookup, contextHelper.ParallelOptions, mapKv =>
            //    // foreach (var mapKV in mapLookup)
            //{
            //    int surfaceStyle;
            //    if (!contextHelper.SurfaceStyles.TryGetValue(mapKv.Key, out surfaceStyle))
            //        //it doesn't have a surface style assigned to itself, so use the base one of the main shape
            //        contextHelper.SurfaceStyles.TryGetValue(mapKv.Value, out surfaceStyle);
            //    GeometryReference geometryReference;
            //    contextHelper.ShapeLookup.TryGetValue(mapKv.Value, out geometryReference);
            //    geometryReference.StyleLabel = surfaceStyle;
            //    contextHelper.ShapeLookup.TryAdd(mapKv.Key, geometryReference);

            //}
            //);
            if (progDelegate != null) progDelegate(101, "WriteShapeGeometries, (" + localTally + " written)");
        }


        private void WriteRegionsToStore(IIfcRepresentationContext context, IEnumerable<XbimBBoxClusterElement> elementsToCluster, IGeometryStoreInitialiser txn)
        {
            //set up a world to partition the model
            var metre = _model.ModelFactors.OneMetre;

            var regions = new XbimRegionCollection();
            // the XbimDBSCAN method adopted for clustering produces clusters of contiguous elements.
            // if the maximum size is a problem they could then be split using other algorithms that divide spaces equally
            //
            var v = XbimDbscan.GetClusters(elementsToCluster, 5 * metre); // .OrderByDescending(x => x.GeometryIds.Count);
            var i = 1;
            regions.AddRange(v.Select(item => new XbimRegion("Region " + i++, item.Bound, item.GeometryIds.Count)));
            regions.ContextLabel = context.EntityLabel;
            txn.AddRegions(regions);
        }

        /// <summary>
        ///     Writes the geometry as a string into the database
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
        ///     Returns an enumerable of all XbimShape Instances in the model in this context, retrieveAllData will ensure that
        ///     bounding box and transformation data are also retrieved
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
        ///     Get the region with the greates population
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
        ///     Returns all instances of the specified shape geometry
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
        ///     Returns the shape instances of the specified product in this context
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
        ///     Returns a triangulated mesh geometry fopr the specified shape
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
        ///     Returns a triangulated mesh geometry fopr the specified shape geometry
        /// </summary>
        /// <returns></returns>
        public IXbimMeshGeometry3D ShapeGeometryMeshOf(XbimShapeGeometry shapeGeometry)
        {
            var mg = new XbimMeshGeometry3D();
            mg.Read(shapeGeometry.ShapeData);
            return mg;
        }

        /// <summary>
        ///     Returns a triangulated mesh geometry fopr the specified shape instance, all transformations are applied
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

