#region Directives

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
using Xbim.Common.XbimExtensions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc2x3.Extensions;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.Kernel;
using Xbim.Ifc2x3.PresentationAppearanceResource;
using Xbim.Ifc2x3.ProductExtension;
using Xbim.Ifc2x3.RepresentationResource;
using Xbim.IO;
using Xbim.ModelGeometry.Scene.Clustering;
using Xbim.XbimExtensions;
using Xbim.XbimExtensions.Interfaces;
using XbimGeometry.Interfaces;

#endregion

namespace Xbim.ModelGeometry.Scene
{
    /// <summary>
    ///     Represents a gemetric representation context, i.e. a 'Body' and 'Model'Representation
    ///     Note a 3DModelContext may contain multiple IfcGeometricRepresentationContexts
    /// </summary>
    public class Xbim3DModelContext
    {
        #region Helper classes

        //private struct to hold details of references to geometries
        private struct GeometryReference
        {
            public XbimRect3D BoundingBox;
            public int GeometryId;
            public int StyleLabel;
        }

        private class IfcRepresentationContextCollection : KeyedCollection<int, IfcRepresentationContext>
        {
            protected override int GetKeyForItem(IfcRepresentationContext item)
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

                    public RepresentationItemGeometricHashKey(IfcRepresentationItem item)
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

                    public IfcRepresentationItem RepresentationItem 
                    {
                        get { return _model.InstancesLocal[_item] as IfcRepresentationItem; }
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

            public XbimCreateContextHelper(XbimModel model, IfcRepresentationContextCollection contexts)
            {
                Model = model;
                Contexts = contexts;
            }
          
            /// <summary>
            /// The key is the Ifc label of the geometry the value is the database record number
            /// </summary>
            internal ConcurrentDictionary<int,int> GeometryShapeLookup { get; private set; }

            private XbimModel Model { get; set; }
            private IfcRepresentationContextCollection Contexts { get; set; }
            internal ParallelOptions ParallelOptions { get; private set; }
            internal XbimPlacementTree PlacementTree { get; private set; }
            internal HashSet<int> MappedShapeIds { get; private set; }
            internal HashSet<int> FeatureElementShapeIds { get; private set; }
            internal List<IGrouping<IfcElement, IfcFeatureElement>> OpeningsAndProjections { get; private set; }
            private HashSet<int> VoidedShapeIds { get; set; }
            internal HashSet<int> ProductShapeIds { get; private set; }
            internal ConcurrentDictionary<int, IXbimGeometryObject> CachedGeometries { get; private set; }
            internal int Total { get; private set; }
            internal int PercentageParsed { get; set; }
            internal int Tally { get; set; }
            internal Dictionary<int, int> SurfaceStyles { get; private set; }

            internal Dictionary<IfcRepresentationContext, ConcurrentQueue<XbimBBoxClusterElement>> Clusters { get; private set;
            }

            internal ConcurrentDictionary<int, List<GeometryReference>> MapsWritten { get; private set; }
            internal ConcurrentDictionary<int, XbimMatrix3D> MapTransforms { get; private set; }

            internal IXbimGeometryObject GetGeometryFromCache(XbimShapeInstance instance)
            {

                int ifcShapeId;
                if (GeometryShapeLookup.TryGetValue(instance.ShapeGeometryLabel, out ifcShapeId))
                {
                    IXbimGeometryObject obj;
                    if (CachedGeometries.TryGetValue(ifcShapeId, out obj))
                        return obj.Transform(instance.Transformation);
                } //it might be a map
                else
                {
                    GeometryReference geomRef;
                    if (ShapeLookup.TryGetValue(instance.InstanceLabel, out geomRef))
                    {
                        IXbimGeometryObject obj;
                        if (CachedGeometries.TryGetValue(geomRef.GeometryId, out obj))
                            return obj.Transform(instance.Transformation);
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
                    VoidedShapeIds = new HashSet<int>();
                    ParallelOptions = new ParallelOptions();
                    //ParallelOptions.MaxDegreeOfParallelism = 8;
                    CachedGeometries = new ConcurrentDictionary<int, IXbimGeometryObject>();
                    foreach (var voidedShapeId in OpeningsAndProjections.Select(op => op.Key.EntityLabel))
                        VoidedShapeIds.Add(voidedShapeId);
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
                Clusters = new Dictionary<IfcRepresentationContext, ConcurrentQueue<XbimBBoxClusterElement>>();
                foreach (var context in Contexts) Clusters.Add(context, new ConcurrentQueue<XbimBBoxClusterElement>());
            }

            private void GetOpeningsAndProjections()
            {
                var openings = Model.InstancesLocal.OfType<IfcRelVoidsElement>()
                    .Where(
                        r =>
                            r.RelatingBuildingElement.Representation != null &&
                            r.RelatedOpeningElement.Representation != null)
                    .Select(
                        f =>
                            new
                            {
                                element = f.RelatingBuildingElement,
                                feature = (IfcFeatureElement) f.RelatedOpeningElement
                            });

                var projections = Model.InstancesLocal.OfType<IfcRelProjectsElement>()
                    .Where(
                        r => r.RelatingElement.Representation != null && r.RelatedFeatureElement.Representation != null)
                    .Select(
                        f => new {element = f.RelatingElement, feature = (IfcFeatureElement) f.RelatedFeatureElement});

                var allOps = openings.Concat(projections);

                OpeningsAndProjections = allOps
                    .GroupBy(x => x.element, y => y.feature).ToList();
            }


            private void GetSurfaceStyles()
            {
                //get all the surface styles

                var styledItemsGroup = Model.InstancesLocal
                    .OfType<IfcStyledItem>()
                    .Where(s => s.Item != null)
                    .GroupBy(s => s.Item.EntityLabel);
                SurfaceStyles = new Dictionary<int, int>();
                foreach (var styledItemGrouping in styledItemsGroup)
                {
                    var val =
                        styledItemGrouping.SelectMany(
                            s => s.Styles.Where(st => st != null).SelectMany(st => st.Styles.OfType<IfcSurfaceStyle>())).FirstOrDefault();
                    if (val!=null)
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
                foreach (var product in Model.InstancesLocal.OfType<IfcProduct>(true).Where(p => p.Representation != null))
                {
                    var isFeatureElementShape = product is IfcFeatureElement ||
                                                VoidedShapeIds.Contains(product.EntityLabel);
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
                            foreach (var shape in rep.Items.Where(i => !(i is IfcGeometricSet)))
                            {
                                var mappedItem = shape as IfcMappedItem;
                                if (mappedItem != null)
                                {
                                    MappedShapeIds.Add(mappedItem.EntityLabel);
                                    //make sure any shapes mapped are in the set to process as well
                                    foreach (var item in mappedItem.MappingSource.MappedRepresentation.Items)
                                    {
                                        if (item != null && !(item is IfcGeometricSet))
                                        {
                                            var mappedItemLabel = item.EntityLabel;
                                            //if not already processed add it
                                            ProductShapeIds.Add(mappedItemLabel);
                                            if (isFeatureElementShape) FeatureElementShapeIds.Add(mappedItemLabel);
                                        }
                                    }
                                }
                                else
                                {
                                    //if not already processed add it
                                    ProductShapeIds.Add(shape.EntityLabel);
                                    if (isFeatureElementShape) FeatureElementShapeIds.Add(shape.EntityLabel);
                                }
                            }
                        }
                    }
                }
            }

            public void Dispose()
            {
                foreach (var cachedGeom in CachedGeometries)
                {
                    cachedGeom.Value.Dispose();
                }
            }
        }

        #endregion

        public static readonly ILogger Logger = LoggerFactory.GetLogger();
        private readonly IfcRepresentationContextCollection _contexts;
        private readonly IXbimGeometryCreator _engine;
        private readonly XbimModel _model;

        private bool _contextIsPersisted;
        //The maximum extent for any dimension of any products bouding box 
        private double _maxXyz;
       


        /// <summary>
        ///     Initialises a model context from the model
        /// </summary>
        /// <param name="model"></param>
        /// <param name="contextType"></param>
        /// <param name="contextIdentifier"></param>
        public Xbim3DModelContext(XbimModel model, string contextType = "model", string contextIdentifier = "body")
        {
            _model = model;
            _engine = new XbimGeometryEngine();
            
           
            
            //Get the required context
            //check for old versions
            var contexts =
                model.InstancesLocal.OfType<IfcGeometricRepresentationContext>()
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
                    model.InstancesLocal.OfType<IfcGeometricRepresentationContext>()
                        .Where(
                            c =>
                                String.Compare(c.ContextType, "design", true) == 0 ||
                                String.Compare(c.ContextType, "model", true) == 0).ToList();
                if (contexts.Any())
                {
                    Logger.InfoFormat(
                        "Unable to find any Geometric Representation contexts with Context Type = {0} and Context Identifier = {1}, using Context Type = 'Design' instead. NB This does not comply with IFC 2x3 or greater, the schema is {2}",
                        contextType, contextIdentifier, string.Join(",", model.Header.FileSchema.Schemas));
                }
                else
                {
                    contexts = model.InstancesLocal.OfType<IfcGeometricRepresentationContext>().ToList();
                    if (contexts.Any())
                    {
                        var ctxtString = contexts.Aggregate("", (current, ctxt) => current + (ctxt.ContextType + " "));
                        if (string.IsNullOrWhiteSpace(ctxtString)) ctxtString = "$";
                        Logger.InfoFormat(
                            "Unable to find any Geometric Representation contexts with Context Type = {0} and Context Identifier = {1}, using  available Context Types '{2}'. NB This does not comply with IFC 2x2 or greater",
                            contextType, contextIdentifier, ctxtString.TrimEnd(' '));
                    }
                    else
                    {
                        Logger.ErrorFormat(
                            "Unable to find any Geometric Representation contexts in this file, it is illegal and does not comply with IFC 2x2 or greater");
                    }
                }
            }

            if (contexts.Any())
            {
                _contexts = new IfcRepresentationContextCollection();
                var roundingPrecisions = new Dictionary<IfcRepresentationContext, int>();
                var contextLookup = new Dictionary<short, IfcRepresentationContext>();
                foreach (var context in contexts)
                {
                    _contexts.Add(context);
                    roundingPrecisions.Add(context,
                        Math.Abs(context.DefaultPrecision) < 1e-9
                            ? 0
                            : Math.Abs((int) Math.Log10(context.DefaultPrecision)));
                    contextLookup.Add(GetContextId(context), context);
                }
                _contextIsPersisted = false;
            }
            
        }

        /// <summary>
        ///     Returns true if the context has been processed and stored in the model, if false call CreateContext
        /// </summary>
        public bool IsGenerated
        {
            get { return _contextIsPersisted; }
        }

        public XbimModel Model
        {
            get { return _model; }
        }

        private static short GetContextId(IfcRepresentationContext context)
        {
            return (short) ((context.EntityLabel >> 16) ^ context.EntityLabel);
        }

        /// <summary>
        /// </summary>
        /// <param name="progDelegate"></param>
        /// <param name="geomStorageType">The type of geometry storage type to use, typically Polyhedron or  PolyhedronBinary</param>
        /// <returns></returns>
        public bool CreateContext(XbimGeometryType geomStorageType = XbimGeometryType.Polyhedron, ReportProgressDelegate progDelegate = null)
        {
            if (_contexts == null || _engine == null) return false;
            if (_contextIsPersisted) return false; //already created it


            using (var contextHelper = new XbimCreateContextHelper(_model, _contexts))
            {
                
            
            if (progDelegate != null) progDelegate(-1, "Initialise");
            if (!contextHelper.Initialise())
                throw new Exception("Failed to initialise geometric context, " + contextHelper.InitialiseError);
            if (progDelegate != null) progDelegate(101, "Initialise");

            using (var shapeGeometries = new BlockingCollection<IXbimShapeGeometryData>())
            {
                // A simple blocking consumer with no cancellation that takes shapes of the queue and stores them in the database
                using (
                    var writeToDb =
                        Task.Factory.StartNew(
// ReSharper disable AccessToDisposedClosure
                            () => WriteShapeGeometriesToDatabase(contextHelper, shapeGeometries)))
// ReSharper restore AccessToDisposedClosure
                {
                    try
                    {  
                        //remove any implicit duplicate geometries by comparing their IFC definitions
                        WriteShapeGeometries(contextHelper, progDelegate, shapeGeometries, geomStorageType);
                    }
                    finally
                    {
                        //wait for the geometry shapes to be written
                        shapeGeometries.CompleteAdding();
                        writeToDb.Wait();
                    }
                }
            }
            
            WriteMappedItems(contextHelper, progDelegate);

                using (var features =
                    new BlockingCollection<Tuple<XbimShapeInstance, IXbimShapeGeometryData>>())
                {
                    //start a new task to process features
                    HashSet<int> processed;
// ReSharper disable once AccessToDisposedClosure
                    using (var writeToDb = Task.Factory.StartNew(() => WriteFeatureElementsToDatabase(features)))
                    {
                        try
                        {
                            processed = WriteFeatureElements(contextHelper, features, progDelegate, geomStorageType);
                        }
                        finally
                        {
                            //wait for the geometry shapes to be written
                            features.CompleteAdding();
                            writeToDb.Wait();
                        }
                    }
                    if (progDelegate != null) progDelegate(-1, "WriteProductShapes");
                    var productsRemaining = _model.InstancesLocal.OfType<IfcProduct>()
                        .Where(p => p.Representation != null && !processed.Contains(p.EntityLabel)).ToList();


                    WriteProductShapes(contextHelper, productsRemaining);
                    if (progDelegate != null) progDelegate(101, "WriteProductShapes");
                    //Write out the actual representation item reference count
                    if (progDelegate != null) progDelegate(-1, "WriteShapeGeometryReferenceCountToDb");
                    WriteShapeGeometryReferenceCountToDb();
                    if (progDelegate != null) progDelegate(101, "WriteShapeGeometryReferenceCountToDb");
                    //Write out the regions of the model
                    if (progDelegate != null) progDelegate(-1, "WriteRegionsToDb");
                    foreach (var cluster in contextHelper.Clusters)
                    {
                        WriteRegionsToDb(cluster.Key, cluster.Value, contextHelper.PlacementTree.WorldCoordinateSystem);
                    }
                    if (progDelegate != null) progDelegate(101, "WriteRegionsToDb");
                    _contextIsPersisted = true;

                    return true;
                }
            }
        }

       

        private void WriteShapeGeometriesToDatabase(XbimCreateContextHelper contextHelper,
            BlockingCollection<IXbimShapeGeometryData> shapeGeometries)
        {
            IXbimShapeGeometryData shapeGeom = null;

            using (var geomTable = _model.GetShapeGeometryTable())
            {

                using (var transaction = geomTable.BeginLazyTransaction())
                {
                    var geomCount = 0;
                    const int transactionBatchSize = 100;

                    while (!shapeGeometries.IsCompleted)
                    {
                        try
                        {
                            if (shapeGeometries.TryTake(out shapeGeom))
                            {
                                var refCounter = new GeometryReference
                                {
                                    BoundingBox = ((XbimShapeGeometry)shapeGeom).BoundingBox,
                                    GeometryId = geomTable.AddGeometry(shapeGeom)
                                    
                                };
                                int styleLabel;
                                contextHelper.SurfaceStyles.TryGetValue(shapeGeom.IfcShapeLabel, out styleLabel);
                                refCounter.StyleLabel = styleLabel;
                                contextHelper.ShapeLookup.TryAdd(shapeGeom.IfcShapeLabel, refCounter);
                                if (contextHelper.CachedGeometries.ContainsKey(shapeGeom.IfcShapeLabel))
                                    //keep a record of the IFC label and database record mapping
                                    contextHelper.GeometryShapeLookup.TryAdd(shapeGeom.ShapeLabel,
                                        shapeGeom.IfcShapeLabel);
                                geomCount++;
                            }
                        }
                        catch (InvalidOperationException)
                        {
                            break;
                        }
                        catch (Exception e)
                        {
                            if (shapeGeom != null)
                                Logger.ErrorFormat("Failed to write entity #{0} to database, error = {1}",
                                    shapeGeom.IfcShapeLabel, e.Message);
                        }
                        long remainder = geomCount % transactionBatchSize; //pulse transactions
                        if (remainder == transactionBatchSize - 1)
                        {
                            transaction.Commit();
                            transaction.Begin();
                        }
                    }
                    transaction.Commit();
                }

            }
        }

        private void WriteFeatureElementsToDatabase(
            BlockingCollection<Tuple<XbimShapeInstance, IXbimShapeGeometryData>> shapeFeatures)
        {
            using (var instanceTable = _model.GetShapeInstanceTable())
            {
                using (var geomTable = _model.GetShapeGeometryTable())
                {

                    using (var instanceTransaction = instanceTable.BeginLazyTransaction())
                    {
                        using (var geomTransaction = geomTable.BeginLazyTransaction())
                        {
                            var instanceCount = 0;
                            const int transactionBatchSize = 100;

                            while (!shapeFeatures.IsCompleted)
                            {
                                try
                                {
                                    Tuple<XbimShapeInstance, IXbimShapeGeometryData> shapeFeature;
                                    if (shapeFeatures.TryTake(out shapeFeature))
                                    {
                                        var shapeInstance = shapeFeature.Item1;
                                        var shapeGeometry = shapeFeature.Item2;
                                        shapeInstance.ShapeGeometryLabel = geomTable.AddGeometry(shapeGeometry);
                                        instanceTable.AddInstance(shapeInstance);
                                    }
                                }
                                catch (InvalidOperationException)
                                {
                                    break;
                                }
                                instanceCount++;
                                long remainder = instanceCount % transactionBatchSize; //pulse transactions
                                if (remainder == transactionBatchSize - 1)
                                {
                                    instanceTransaction.Commit();
                                    instanceTransaction.Begin();
                                    geomTransaction.Commit();
                                    geomTransaction.Begin();
                                }
                            }
                            geomTransaction.Commit();
                        }
                        instanceTransaction.Commit();
                    }

                }
            }
        }

        private HashSet<int> WriteFeatureElements(XbimCreateContextHelper contextHelper,
            BlockingCollection<Tuple<XbimShapeInstance, IXbimShapeGeometryData>> features,
            ReportProgressDelegate progDelegate, XbimGeometryType geomType
            )
        {
            var processed = new HashSet<int>();
            var localPercentageParsed = contextHelper.PercentageParsed;
            var localTally = contextHelper.Tally;

            if (progDelegate != null) progDelegate(-1, "WriteFeatureElements (" + contextHelper.OpeningsAndProjections.Count + " elements)");
            
            Parallel.ForEach(contextHelper.OpeningsAndProjections,contextHelper.ParallelOptions, pair =>
            //        foreach (IGrouping<IfcElement, IfcFeatureElement> pair in contextHelper.OpeningsAndProjections)
            {

                var element = pair.Key;
               // if (element.EntityLabel == 5761624)
                {
                    Interlocked.Increment(ref localTally);
                   
                    try
                    {
                        var elementShapes = WriteProductShape(contextHelper, element, false).ToList();
                        var context = 0;
                        var styleId = 0; //take the style of any part of the main shape
                        if (elementShapes.Any())
                        {

                            //Get all the parts of this element into a set of solid geometries
                            var elementGeom = _engine.CreateSolidSet();
                            foreach (var elemShape in elementShapes)
                            {
                                var geom = contextHelper.GetGeometryFromCache(elemShape);

                                elementGeom.Add(geom);
                                context = elemShape.RepresentationContext;
                                if (elemShape.StyleLabel > 0) styleId = elemShape.StyleLabel;
                            }
                            if (elementGeom.IsSimplified)
                                Logger.WarnFormat(
                                    "WM009: Geometry of object #{0} '{1}' [{2}] is too complex and it will make interoperability difficult for you. It has been simplified, no openings have been cut. You should consider re-authoring it in your BIM tool",
                                    element.EntityLabel, element.Name, element.GetType().Name);
                            if (elementGeom.Count == 0)
                            {
                                Logger.WarnFormat(
                                    "WM003: An element #{0} does not have a valid solid representation, openings cannot be formed",
                                    element.EntityLabel);

                            }
                            if (elementGeom.Count > 0 && !elementGeom.IsSimplified)
                            {
                                //now build the openings
                                var allOpenings = _engine.CreateSolidSet();
                                var allProjections = _engine.CreateSolidSet();

                                foreach (var feature in pair)
                                {
                                    var opening = feature as IfcFeatureElementSubtraction;
                                    if (opening != null)
                                    {
                                        var openingShapes = WriteProductShape(contextHelper, opening, false).ToList();

                                        foreach (var openingShape in openingShapes)
                                        {
                                            var openingGeom = contextHelper.GetGeometryFromCache(openingShape);
                                            if (openingGeom != null)
                                                allOpenings.Add(openingGeom);
                                        }

                                        if (allOpenings.Count == 0)
                                        {
                                            Logger.WarnFormat(
                                                "WM004: {0} - #{1} is an opening that has been no 3D geometric form definition",
                                                opening.GetType().Name, opening.EntityLabel);
                                        }
                                        processed.Add(opening.EntityLabel);
                                    }
                                    else
                                    {
                                        var addition = feature as IfcFeatureElementAddition;
                                        if (addition != null)
                                        {
                                            var projectionShapes =
                                                WriteProductShape(contextHelper, addition, false).ToList();
                                            foreach (var projectionShape in projectionShapes)
                                            {
                                                var projGeom = contextHelper.GetGeometryFromCache(projectionShape);
                                                if (projGeom != null)
                                                    allProjections.Add(projGeom);
                                            }
                                            if (allProjections.Count == 0)
                                            {
                                                Logger.WarnFormat(
                                                    "WM005: {0} - #{1} is an projection that has been no 3D geometric form definition",
                                                    addition.GetType().Name, addition.EntityLabel);
                                            }
                                            processed.Add(addition.EntityLabel);
                                        }
                                    }
                                }

                                //make the finished shape
                                if (allProjections.Any())
                                    elementGeom = elementGeom.Union(allProjections, _model.ModelFactors.PrecisionBoolean);

                                if (allOpenings.Any())
                                    elementGeom = elementGeom.Cut(allOpenings, _model.ModelFactors.PrecisionBoolean);
                                if (elementGeom.IsSimplified)
                                    Logger.WarnFormat(
                                        "WM008: Geometry of object #{0} '{1}' [{2}] is too complex and it will make interoperability difficult for you. It has been simplified, small openings have not been cut. You should consider re-authoring it in your BIM tool",
                                        element.EntityLabel, element.Name, element.GetType().Name);

                            }
                            ////now add to the DB               
                            XbimModelFactors mf = _model.ModelFactors;
                            foreach (var geom in elementGeom)
                            {
                                IXbimShapeGeometryData shapeGeometry = new XbimShapeGeometry
                                {
                                    IfcShapeLabel = element.EntityLabel,
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
                                        _engine.WriteTriangulation(bw, geom, mf.Precision, mf.DeflectionTolerance,
                                            mf.DeflectionAngle);
                                    }
                                }
                                else
                                {
                                    using (var tw = new StreamWriter(memStream))
                                    {
                                        _engine.WriteTriangulation(tw, geom, mf.Precision, mf.DeflectionTolerance,
                                            mf.DeflectionAngle);
                                    }
                                }
                                shapeGeometry.ShapeData = memStream.ToArray();
                                var shapeInstance = new XbimShapeInstance
                                {
                                    IfcProductLabel = element.EntityLabel,
                                    ShapeGeometryLabel = 0,
                                    /*Set when geometry written*/
                                    StyleLabel = styleId,
                                    RepresentationType = XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded,
                                    RepresentationContext = context,
                                    IfcTypeId = IfcMetaData.IfcTypeId(element),
                                    Transformation = XbimMatrix3D.Identity,
                                    BoundingBox = elementGeom.BoundingBox
                                };
                                features.Add(new Tuple<XbimShapeInstance, IXbimShapeGeometryData>(shapeInstance,
                                    shapeGeometry));
                            }
                        }
                        else
                            Logger.ErrorFormat(
                                "WM006: #{1} - {0} is an element that contains openings and has an invalid 3D geometric form definition",
                                element.GetType().Name, element.EntityLabel);
                        processed.Add(element.EntityLabel);
                        if (progDelegate != null)
                        {
                            var newPercentage = Convert.ToInt32((double) localTally/contextHelper.Total*100.0);
                            if (newPercentage > localPercentageParsed)
                            {
                                Interlocked.Exchange(ref localPercentageParsed, newPercentage);
                                progDelegate(localPercentageParsed, "Building Elements");
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        Logger.WarnFormat(
                            "WM007: {1} - {0} is an element that contains openings but it has a bad 3D geometric form definition, {2}",
                            element.GetType().Name, element.EntityLabel, e.Message);
                    }
                }
            }
               );
            contextHelper.PercentageParsed = localPercentageParsed;
            contextHelper.Tally = localTally;
            if (progDelegate != null) progDelegate(101, "WriteFeatureElements, (" + localTally + " written)");
            return processed;
        }

        private void WriteProductShapes(XbimCreateContextHelper contextHelper, IEnumerable<IfcProduct> products)
        {
            var localTally = contextHelper.Tally;
            var localPercentageParsed = contextHelper.PercentageParsed;

            Parallel.ForEach(products, contextHelper.ParallelOptions, product =>
                //    foreach (var product in products)
            {
                //select representations that are in the required context
                //only want solid representations for this context, but rep type is optional so just filter identified 2d elements
                //we can only handle one representation in a context and this is in an implementers agreement
                var rep = product.Representation.Representations.FirstOrDefault(r => _contexts.Contains(r.ContextOfItems) &&
                                                                                     r.IsBodyRepresentation());
                //write out the representation if it has one
                if (rep != null)
                {
                    WriteProductShape(contextHelper, product, true);
                }
            }
                );
            contextHelper.Tally = localTally;
            contextHelper.PercentageParsed = localPercentageParsed;
        }

        /// <summary>
        ///     Process the products shape and writes the instances of shape geometries to the Database
        /// </summary>
        /// <param name="contextHelper"></param>
        /// <param name="element">Element to write</param>
        /// <param name="includesOpenings"></param>
        /// <returns>IEnumerable of XbimShapeInstance that have been written</returns>
        private IEnumerable<XbimShapeInstance> WriteProductShape(XbimCreateContextHelper contextHelper,
            IfcProduct element, bool includesOpenings)
        {
            _maxXyz = _model.ModelFactors.OneMetre * 100; //elements bigger than 100 metres should not be considered in region
            var shapesInstances = new List<XbimShapeInstance>();
            var rep = element.Representation.Representations
                .FirstOrDefault(r => _contexts.Contains(r.ContextOfItems)
                                     && r.IsBodyRepresentation());
            if (rep == null) return Enumerable.Empty<XbimShapeInstance>();

            var placementTransform = contextHelper.PlacementTree[element.ObjectPlacement.EntityLabel];
            var contextId = rep.ContextOfItems.EntityLabel;

            //write out any shapes it has                   
            foreach (var shape in rep.Items)
            {
                if (shape is IfcMappedItem)
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
                                WriteShapeInstanceToDb(instance.GeometryId, instance.StyleLabel, contextId,
                                    element,
                                    trans, instance.BoundingBox /*productBounds*/,
                                    includesOpenings
                                        ? XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded
                                        : XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded)
                                );
                            var transproductBounds = instance.BoundingBox /*productBounds*/.Transform(placementTransform);
                            //transform the bounds

                            //if (transproductBounds.SizeX < _maxXyz && transproductBounds.SizeY < _maxXyz && transproductBounds.SizeZ < _maxXyz)
                            contextHelper.Clusters[rep.ContextOfItems].Enqueue(
                            new XbimBBoxClusterElement(instance.GeometryId,
                                transproductBounds));
                        }
                    }
                }
                else //it is a direct reference to geometry shape
                {
                    GeometryReference instance;
                    if (contextHelper.ShapeLookup.TryGetValue(shape.EntityLabel, out instance))
                    {
                        shapesInstances.Add(
                            WriteShapeInstanceToDb(instance.GeometryId, instance.StyleLabel, contextId, element,
                                placementTransform, instance.BoundingBox /*productBounds*/,
                                includesOpenings
                                    ? XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded
                                    : XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded)
                            );
                        var transproductBounds = instance.BoundingBox /*productBounds*/.Transform(placementTransform);
                        //transform the bounds
                        contextHelper.Clusters[rep.ContextOfItems].Enqueue(
                            new XbimBBoxClusterElement(instance.GeometryId,
                                transproductBounds));
                    }
                    //else
                    //    Logger.ErrorFormat("Failed to find shape #{0}", shape.EntityLabel);
                }
            }
            return shapesInstances;
        }

        private void WriteMappedItems(XbimCreateContextHelper contextHelper, ReportProgressDelegate progDelegate)
        {
            if (progDelegate != null) progDelegate(-1, "WriteMappedItems (" + contextHelper.MappedShapeIds.Count + " items)");
            Parallel.ForEach(contextHelper.MappedShapeIds, contextHelper.ParallelOptions, mapId =>
                //   foreach (var map in allMaps)
            {
                var entity = _model.InstancesLocal[mapId];
                var map = entity as IfcMappedItem;
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
                        else if (!(mapShape is IfcGeometricSet)) //ignore non solid geometry sets
                            Logger.ErrorFormat("Failed to find shape #{0}", mapShape.EntityLabel);
                    }
                    if (mapShapes.Any()) //if we have something to write
                    {
                        contextHelper.MapsWritten.TryAdd(map.EntityLabel, mapShapes);
                        var cartesianTransform = map.MappingTarget.ToMatrix3D();
                        var localTransform = map.MappingSource.MappingOrigin.ToMatrix3D();
                        contextHelper.MapTransforms.TryAdd(map.EntityLabel,
                            XbimMatrix3D.Multiply(cartesianTransform, localTransform));
                    }
                }
                else
                    Logger.ErrorFormat("Illegal entity found in maps collection #{0}, type {1}", entity.EntityLabel,
                        entity.GetType().Name);
            }
                );
            if (progDelegate != null) progDelegate(101, "WriteMappedItems, (" + contextHelper.MappedShapeIds.Count + " written)");
        }
 

        private void WriteShapeGeometries(XbimCreateContextHelper contextHelper, ReportProgressDelegate progDelegate, BlockingCollection<IXbimShapeGeometryData> shapeGeometries, XbimGeometryType geomStorageType)
        {
            var localPercentageParsed = contextHelper.PercentageParsed;
            var localTally = contextHelper.Tally;
           // var dedupCount = 0;
            var xbimTessellator = new XbimTessellator(Model,geomStorageType);
            //var geomHash = new ConcurrentDictionary<RepresentationItemGeometricHashKey, int>();
            
            var mapLookup =
                new ConcurrentDictionary<int, int>();
            if (progDelegate != null) progDelegate(-1, "WriteShapeGeometries (" + contextHelper.ProductShapeIds.Count + " shapes)");
            var precision = Model.ModelFactors.Precision;
            var deflection = Model.ModelFactors.DeflectionTolerance;
            var deflectionAngle = Model.ModelFactors.DeflectionAngle;
           Parallel.ForEach(contextHelper.ProductShapeIds, contextHelper.ParallelOptions, shapeId =>
         // foreach (var shapeId in contextHelper.ProductShapeIds)
            {
                Interlocked.Increment(ref localTally);
                var shape = (IfcGeometricRepresentationItem)Model.InstancesLocal[shapeId];
              //  var key = new RepresentationItemGeometricHashKey(shape);
                var isFeatureElementShape = contextHelper.FeatureElementShapeIds.Contains(shapeId);
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
                    // Console.WriteLine(((IfcFacetedBrep) shape).Outer.CfsFaces.Count);
                    try
                    {
                        // Console.WriteLine(shape.GetType().Name);
                        // using (var geomModel = _engine.Create(shape))
                        {
                            IXbimShapeGeometryData shapeGeom = null;
                            IXbimGeometryObject geomModel = null;
                            if (!isFeatureElementShape && xbimTessellator.CanMesh(shape)) //if we can mesh the shape directly just do it
                            {
                                shapeGeom = xbimTessellator.Mesh(shape);
                            }
                            else //we need to create a geometry object
                            {
                                geomModel = _engine.Create(shape);
                                if (geomModel != null && geomModel.IsValid)
                                    shapeGeom = _engine.CreateShapeGeometry(geomModel,precision,deflection,deflectionAngle,geomStorageType);
                            }

                            if (shapeGeom == null || shapeGeom.ShapeData==null || shapeGeom.ShapeData.Length == 0)
                                Logger.WarnFormat("WM001: Entity #{0} of type {1} is an empty shape", shapeId,
                                    shape.GetType().Name);
                            else
                            {
                                shapeGeom.IfcShapeLabel = shapeId;
                                shapeGeometries.Add(shapeGeom);
                            }
                            if (geomModel != null && geomModel.IsValid)
                            {
                                if (isFeatureElementShape)
                                    //we need for boolean operations later, add the polyhedron if the face is planar
                                    contextHelper.CachedGeometries.TryAdd(shapeId, geomModel);
                                else
                                    geomModel.Dispose();
                            }
                            
                        }
                    }
                    catch (Exception e)
                    {
                        Logger.ErrorFormat("WM002: Failed to add shape geometry for entity #{0}, reason = {1}", shapeId,
                            e.Message);
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
            Parallel.ForEach(mapLookup, contextHelper.ParallelOptions, mapKv =>
                // foreach (var mapKV in mapLookup)
            {
                int surfaceStyle;
                if (!contextHelper.SurfaceStyles.TryGetValue(mapKv.Key, out surfaceStyle))
                    //it doesn't have a surface style assigned to itself, so use the base one of the main shape
                    contextHelper.SurfaceStyles.TryGetValue(mapKv.Value, out surfaceStyle);
                GeometryReference geometryReference;
                contextHelper.ShapeLookup.TryGetValue(mapKv.Value, out geometryReference);
                geometryReference.StyleLabel = surfaceStyle;
                contextHelper.ShapeLookup.TryAdd(mapKv.Key, geometryReference);
               
            }
            );
            if (progDelegate != null) progDelegate(101, "WriteShapeGeometries, (" + localTally + " written)");
        }


        private void WriteRegionsToDb(IfcRepresentationContext context, IEnumerable<XbimBBoxClusterElement> elementsToCluster, XbimMatrix3D wcsMatrix3D)
        {
            //set up a world to partition the model
            var metre = _model.ModelFactors.OneMetre;
            using (var geomTable = Model.GetGeometryTable())
            {
                var regions = new XbimRegionCollection();
                // the XbimDBSCAN method adopted for clustering produces clusters of contiguous elements.
                // if the maximum size is a problem they could then be split using other algorithms that divide spaces equally
                //
                var v = XbimDbscan.GetClusters(elementsToCluster, 5 * metre); // .OrderByDescending(x => x.GeometryIds.Count);
                var i = 1;
                regions.AddRange(v.Select(item => new XbimRegion("Region " + i++, item.Bound, item.GeometryIds.Count)));


                var transaction = geomTable.BeginLazyTransaction();
                geomTable.AddGeometry(context.EntityLabel, XbimGeometryType.Region,
                    IfcMetaData.IfcTypeId(context.GetType()), wcsMatrix3D.ToArray(), regions.ToArray());
                transaction.Commit();

            }
        }

        private void WriteShapeGeometryReferenceCountToDb()
        {
            //Get the meta data about the instances
            var counts = ShapeInstances().GroupBy(
                i => i.ShapeGeometryLabel,
                (label, instances) => new
                {
                    Label = label,
                    Count = instances.Count()
                });
            using (var geomTable = _model.GetShapeGeometryTable())
            {
                try
                {
                    var transaction = geomTable.BeginLazyTransaction();
                    foreach (var item in counts)
                    {
                        geomTable.UpdateReferenceCount(item.Label, item.Count);
                    }
                    transaction.Commit();
                }
                catch (Exception)
                {
                    Logger.ErrorFormat("Failed to update reference count on geometry");
                }
            }
        }


        /// <summary>
        ///     Writes the geometry as a string into the database
        /// </summary>
        /// <returns></returns>
        private XbimShapeInstance WriteShapeInstanceToDb( int shapeLabel, int styleLabel, int ctxtId,
            IfcProduct product, XbimMatrix3D placementTransform, XbimRect3D bounds,
            XbimGeometryRepresentationType repType)
        {
            var shapeInstance = new XbimShapeInstance
            {
                IfcProductLabel = product.EntityLabel,
                ShapeGeometryLabel = shapeLabel,
                StyleLabel = styleLabel,
                RepresentationType = repType,
                RepresentationContext = ctxtId,
                IfcTypeId = IfcMetaData.IfcTypeId(product),
                Transformation = placementTransform,
                BoundingBox = bounds
            };
           using (var geomTable = Model.GetShapeInstanceTable())
            {
                try
                {
                    using (var transaction = geomTable.BeginLazyTransaction())
                    {
                        geomTable.AddInstance(shapeInstance);
                        transaction.Commit(); 
                    }
                }
                catch (Exception e)
                {
                    Logger.ErrorFormat("Failed to add product geometry for entity #{0}, reason {1}", product.EntityLabel,
                        e.Message);
                }
                
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

            using (var shapeInstanceTable = _model.GetShapeInstanceTable())
            {

                using (shapeInstanceTable.BeginReadOnlyTransaction())
                {
                    foreach (var context in _contexts)
                    {
                        IXbimShapeInstanceData shapeInstance = new XbimShapeInstance();
                        if (shapeInstanceTable.TrySeekShapeInstance(context.EntityLabel, ref shapeInstance))
                        {
                            do
                            {
                                yield return (XbimShapeInstance)shapeInstance;
                            } while (shapeInstanceTable.TryMoveNextShapeInstance(ref shapeInstance));
                        }
                    }

                }

            }
        }

        /// <summary>
        ///     Retunrs shape instances grouped by their style there is very little overhead to this function when compared to
        ///     calling ShapeInstances
        /// </summary>
        /// <param name="incorporateFeatures">
        ///     if true openings and projections are applied to shapes and not returned separately,
        ///     if false openings and shapes that contain openings, projections and shapes that would incorporate these are
        ///     returned as separate entities without incorporation
        /// </param>
        /// <returns></returns>
        public IEnumerable<IGrouping<int, XbimShapeInstance>> ShapeInstancesGroupByStyle(bool incorporateFeatures = true)
        {
            //note the storage of the instances in the DB is ordered by context and then style and type Id so the natural order will group correctly
            long currentStyle = 0;
            XbimShapeInstanceStyleGrouping grp = null;
            int openingLabel = IfcMetaData.IfcTypeId(typeof (IfcOpeningElement));
            int projectionLabel = IfcMetaData.IfcTypeId(typeof (IfcProjectionElement));

            var groupedInstances = new List<XbimShapeInstance>();
            foreach (var instance in ShapeInstances())
            {
                long nextStyle = instance.StyleLabel;
                //use a negative type Id if there is no style defined for this object and render on type
                if (nextStyle == 0) nextStyle = -(instance.IfcTypeId);
                if (currentStyle != nextStyle) //we have the beginnings of a group
                {
                    if (grp != null) //it is not the first time
                    {
                        yield return grp; //return a populated group
                    }
                    groupedInstances = new List<XbimShapeInstance>();
                    grp = new XbimShapeInstanceStyleGrouping((int) nextStyle, groupedInstances);
                    currentStyle = nextStyle;
                }

                if (incorporateFeatures && instance.IfcTypeId != openingLabel &&
                    instance.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded &&
                    instance.IfcTypeId != projectionLabel)
                    groupedInstances.Add(instance);
                else if (!incorporateFeatures &&
                         instance.RepresentationType != XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded)
                    groupedInstances.Add(instance);
            }
            //finally return the current group if not null
            if (grp != null) //it is not the first time
            {
                yield return grp; //return a populated group
            }
        }

        public IEnumerable<XbimShapeGeometry> ShapeGeometries()
        {

            using (var shapeGeometryTable = Model.GetShapeGeometryTable())
            {

                using (shapeGeometryTable.BeginReadOnlyTransaction())
                {
                    IXbimShapeGeometryData shapeGeometry = new XbimShapeGeometry();
                    if (shapeGeometryTable.TryMoveFirstShapeGeometry(ref shapeGeometry))
                    {
                        do
                        {
                            yield return (XbimShapeGeometry)shapeGeometry;
                        } while (shapeGeometryTable.TryMoveNextShapeGeometry(ref shapeGeometry));
                    }

                }

            }
        }


        public XbimShapeGeometry ShapeGeometry(int shapeGeometryLabel)
        {

            using (var shapeGeometryTable = Model.GetShapeGeometryTable())
            {

                using (shapeGeometryTable.BeginReadOnlyTransaction())
                {
                    IXbimShapeGeometryData shapeGeometry = new XbimShapeGeometry();
                    shapeGeometryTable.TryGetShapeGeometry(shapeGeometryLabel, ref shapeGeometry);

                    return (XbimShapeGeometry)shapeGeometry;
                }

            }
        }

        public XbimShapeGeometry ShapeGeometry(XbimShapeInstance shapeInstance)
        {
            return ShapeGeometry(shapeInstance.ShapeGeometryLabel);
        }

        /// <summary>
        ///     Returns an enumerable of all the unique surface styles used in this context, optionally pass in a colour map to
        ///     obtain styles defaulted by type where they have no definition in the model
        /// </summary>
        /// <returns></returns>
        public IEnumerable<XbimTexture> SurfaceStyles(XbimColourMap colourMap = null)
        {
            if (colourMap == null) colourMap = new XbimColourMap();
            var productTypes = new HashSet<short>();
            
            using (var shapeInstanceTable = _model.GetShapeInstanceTable())
            {

                using (shapeInstanceTable.BeginReadOnlyTransaction())
                {
                    foreach (var context in _contexts)
                    {
                        int surfaceStyle;
                        short productType;
                        if (shapeInstanceTable.TryMoveFirstSurfaceStyle(context.EntityLabel, out surfaceStyle,
                            out productType))
                        {
                            do
                            {
                                if (surfaceStyle > 0) //we have a surface style
                                {
                                    var ss = (IfcSurfaceStyle)_model.InstancesLocal[surfaceStyle];
                                    yield return new XbimTexture().CreateTexture(ss);
                                    surfaceStyle = shapeInstanceTable.SkipSurfaceStyes(surfaceStyle);
                                }
                                else //then we use the product type for the surface style
                                {
                                    //read all shape instance of style 0 and get their product texture
                                    do
                                    {
                                        if (productTypes.Add(productType)) //if we have not seen this yet then add it
                                        {
                                            var theType = IfcMetaData.GetType(productType);
                                            var texture = new XbimTexture().CreateTexture(colourMap[theType.Name]);
                                            //get the colour to use
                                            texture.DefinedObjectId = productType * -1;
                                            yield return texture;
                                        }
                                    } while (
                                        shapeInstanceTable.TryMoveNextSurfaceStyle(out surfaceStyle, out productType) &&
                                        surfaceStyle == 0); //skip over all the zero entriesand get theeir style
                                }
                            } while (surfaceStyle != -1);
                            //now get all the undefined styles and use their product type to create the texture
                        }
                    }

                }

            }
        }


        public IEnumerable<XbimRegion> GetRegions()
        {
            if (_contexts == null) return null; //nothing to do
            var regionDataColl = new List<XbimGeometryData>();
            foreach (var context in _contexts)
            {
                regionDataColl.AddRange(_model.GetGeometryData(context.EntityLabel, XbimGeometryType.Region));
            }

            if (regionDataColl.Any())
            {
                var regions = new XbimRegionCollection();
                foreach (var regionData in regionDataColl)
                {
                    regions.AddRange(XbimRegionCollection.FromArray(regionData.ShapeData));
                }
                return regions;
            }
            return Enumerable.Empty<XbimRegion>();
        }

        /// <summary>
        ///     Get the region with the greates population
        /// </summary>
        /// <returns></returns>
        public XbimRegion GetLargestRegion()
        {
            if (_contexts == null) return null; //nothing to do
            var regionDataColl = new List<XbimGeometryData>();
            foreach (var context in _contexts)
                regionDataColl.AddRange(_model.GetGeometryData(context.EntityLabel, XbimGeometryType.Region));

            if (!regionDataColl.Any()) return null;
            var regions = new XbimRegionCollection();
            foreach (var regionData in regionDataColl)
            {
                var wcsMatrix3D = XbimMatrix3D.FromArray(regionData.DataArray2);
                var regColl = XbimRegionCollection.FromArray(regionData.ShapeData);
                foreach (var region in regColl) region.WorldCoordinateSystem = wcsMatrix3D;
                regions.AddRange(regColl);
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
         
            using (var shapeInstanceTable = _model.GetShapeInstanceTable())
            {

                using (shapeInstanceTable.BeginReadOnlyTransaction())
                {
                    foreach (var context in _contexts)
                    {
                        IXbimShapeInstanceData shapeInstance = new XbimShapeInstance();
                        if (shapeInstanceTable.TrySeekShapeInstanceOfGeometry(geometry.ShapeLabel, ref shapeInstance))
                        {
                            do
                            {
                                if (context.EntityLabel == shapeInstance.RepresentationContext &&
                                    shapeInstance.RepresentationType != (byte)XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded &&
                                    !(ignoreFeatures && typeof(IfcFeatureElement).IsAssignableFrom(IfcMetaData.GetType(shapeInstance.IfcTypeId))))
                                    yield return (XbimShapeInstance)shapeInstance;
                            } while (shapeInstanceTable.TryMoveNextShapeInstance(ref shapeInstance));
                        }
                    }

                }

            }
        }

        public IEnumerable<XbimShapeInstance> ShapeInstancesOf(int geometryLabel, bool ignoreFeatures = false)
        {

            using (var shapeInstanceTable = _model.GetShapeInstanceTable())
            {

                using (shapeInstanceTable.BeginReadOnlyTransaction())
                {
                    foreach (var context in _contexts)
                    {
                        IXbimShapeInstanceData shapeInstance = new XbimShapeInstance();
                        if (shapeInstanceTable.TrySeekShapeInstanceOfGeometry(geometryLabel, ref shapeInstance))
                        {
                            do
                            {

                                if (context.EntityLabel == shapeInstance.RepresentationContext &&
                                    shapeInstance.RepresentationType != (byte)XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded &&
                                    !(ignoreFeatures && typeof(IfcFeatureElement).IsAssignableFrom(IfcMetaData.GetType(shapeInstance.IfcTypeId))))
                                    yield return (XbimShapeInstance)shapeInstance;
                                shapeInstance = new XbimShapeInstance();
                            } while (shapeInstanceTable.TryMoveNextShapeInstance(ref shapeInstance));
                        }
                    }

                }

            }
        }

        /// <summary>
        ///     Returns the shape instances of the specified product in this context
        /// </summary>
        /// <param name="product"></param>
        /// <returns></returns>
        public IEnumerable<XbimShapeInstance> ShapeInstancesOf(IfcProduct product)
        {

            using (var shapeInstanceTable = _model.GetShapeInstanceTable())
            {

                using (shapeInstanceTable.BeginReadOnlyTransaction())
                {
                    foreach (var context in _contexts)
                    {
                        IXbimShapeInstanceData shapeInstance = new XbimShapeInstance();
                        if (shapeInstanceTable.TrySeekShapeInstanceOfProduct(product.EntityLabel, ref shapeInstance))
                        {
                            do
                            {
                                if (context.EntityLabel == shapeInstance.RepresentationContext)
                                    yield return (XbimShapeInstance)shapeInstance;
                            } while (shapeInstanceTable.TryMoveNextShapeInstance(ref shapeInstance));
                        }
                    }

                }

            }
        }

        /// <summary>
        ///     Returns the shape instances that have a surface style of the  specified texture in this context
        /// </summary>
        /// <returns></returns>
        public IEnumerable<XbimShapeInstance> ShapeInstancesOf(XbimTexture texture)
        {
            
            using (var shapeInstanceTable = _model.GetShapeInstanceTable())
            {

                using (shapeInstanceTable.BeginReadOnlyTransaction())
                {
                    foreach (var context in _contexts)
                    {
                        IXbimShapeInstanceData shapeInstance = new XbimShapeInstance();

                        if (texture.DefinedObjectId > 0)
                        {
                            if (shapeInstanceTable.TrySeekSurfaceStyle(context.EntityLabel, texture.DefinedObjectId,
                                ref shapeInstance))
                            {
                                do
                                {
                                    yield return (XbimShapeInstance)shapeInstance;
                                } while (shapeInstanceTable.TryMoveNextShapeInstance(ref shapeInstance) &&
                                         shapeInstance.StyleLabel == texture.DefinedObjectId);
                            }
                        }
                        else if (texture.DefinedObjectId < 0)
                            //if the texture is for a type then get all instances of the type
                        {
                            var typeId = (short)Math.Abs(texture.DefinedObjectId);
                            if (shapeInstanceTable.TrySeekProductType(typeId, ref shapeInstance))
                            {
                                do
                                {
                                    if (context.EntityLabel == shapeInstance.RepresentationContext)
                                        yield return (XbimShapeInstance)shapeInstance;
                                } while (shapeInstanceTable.TryMoveNextShapeInstance(ref shapeInstance) &&
                                         shapeInstance.IfcTypeId == typeId);
                            }
                        }
                    }

                }

            }
        }

        /// <summary>
        ///     Returns a lookup of each geometry label and the number of instances that reference it
        /// </summary>
        /// <returns></returns>
        public IDictionary<int, int> ShapeGeometryReferenceData()
        {

            var lookup = new Dictionary<int, int>();
            using (var shapeGeometryTable = Model.GetShapeGeometryTable())
            {

                using (shapeGeometryTable.BeginReadOnlyTransaction())
                {
                    if (shapeGeometryTable.TryMoveFirstReferenceCounter())
                    {
                        do
                        {
                            lookup.Add(shapeGeometryTable.GetShapeGeometryLabel(),
                                shapeGeometryTable.GetReferenceCount());
                        } while (shapeGeometryTable.TryMoveNextReferenceCounter());
                    }

                }

            }
            return lookup;
        }


        /// <summary>
        ///     Returns whether the product has geometry
        /// </summary>
        /// <returns></returns>
        public bool ProductHasGeometry(Int32 productId)
        {
            using (var shapeInstanceTable = _model.GetShapeInstanceTable())
            using (shapeInstanceTable.BeginReadOnlyTransaction())
                return _contexts.Any(context => shapeInstanceTable.TrySeekShapeInstanceOfProduct(productId));
        }

        /// <summary>
        ///     Returns whether the product has geometry
        /// </summary>
        /// <param name="product"></param>
        /// <returns></returns>
        public bool ProductHasGeometry(IfcProduct product)
        {
            return ProductHasGeometry(product.EntityLabel);
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
                shapeInstance.InstanceLabel, shapeInstance.Transformation, _model.UserDefinedId);
            return mg;
        }

        /// <summary>
        ///     Creates a SceneJS scene on the textwriter for the model context
        /// </summary>
        /// <param name="textWriter"></param>
        public void CreateSceneJs(TextWriter textWriter)
        {
           
            using (JsonWriter writer = new JsonTextWriter(textWriter))
            {
                var sceneJs = new XbimSceneJS(this);
                sceneJs.WriteScene(writer);
            }
        }

        public string CreateSceneJs()
        {
            var sw = new StringWriter();
            CreateSceneJs(sw);
            return sw.ToString();
        }

        public void Write(BinaryWriter binaryStream)
        {
        // ReSharper disable RedundantCast
            var lookup = ShapeGeometryReferenceData();
            var styles = SurfaceStyles().ToList();
            var regions = GetRegions().ToList();

            int numberOfGeometries = 0;
            int numberOfVertices = 0;
            int numberOfTriangles = 0;
            int numberOfMatrices = 0;
            int numberOfProducts = 0;
            int numberOfStyles = styles.Count;
            //start writing out

            binaryStream.Write((Int32)94132117); //magic number

            binaryStream.Write((byte)1); //version of stream
            var start = (int)binaryStream.Seek(0, SeekOrigin.Current);
            binaryStream.Write((Int32)0); //number of shapes
            binaryStream.Write((Int32)0); //number of vertices
            binaryStream.Write((Int32)0); //number of triangles
            binaryStream.Write((Int32)0); //number of matrices
            binaryStream.Write((Int32)0); //number of products
            binaryStream.Write((Int32)numberOfStyles); //number of styles
            binaryStream.Write(Convert.ToSingle(_model.ModelFactors.OneMetre)); //write out conversion to meter factor
    
            binaryStream.Write(Convert.ToInt16(regions.Count)); //write out the population data
            foreach (var r in regions)
            {
                binaryStream.Write((Int32)(r.Population));
                var bounds = r.ToXbimRect3D();
                var centre = r.Centre;
                //write out the centre of the region
                binaryStream.Write((Single)centre.X);
                binaryStream.Write((Single)centre.Y);
                binaryStream.Write((Single)centre.Z);
                //bounding box of largest region
                binaryStream.Write(bounds.ToFloatArray());
            }
            //textures
            foreach (var texture in styles)
            {              
                if (texture.ColourMap.Count == 1) //just take the first colour
                {
                    binaryStream.Write((Int32)texture.DefinedObjectId); //style ID
                    XbimColour colour = texture.ColourMap[0];
                    binaryStream.Write((Single)colour.Red);
                    binaryStream.Write((Single)colour.Green);
                    binaryStream.Write((Single)colour.Blue);
                    binaryStream.Write((Single)colour.Alpha);
                }
            }

            //write out all the product bounding boxes
            foreach (var product in _model.InstancesLocal.OfType<IfcProduct>())
            {
                if (!(product is IfcFeatureElement))
                {
                    XbimRect3D bb = XbimRect3D.Empty;
                    foreach (var si in ShapeInstancesOf(product))
                    {
                        var bbPart = XbimRect3D.TransformBy(si.BoundingBox, si.Transformation); //make sure we put the box in the right place and then convert to axis aligned
                        if (bb.IsEmpty) bb = bbPart;
                        else
                            bb.Union(bbPart);
                    }
                    if (!bb.IsEmpty) //do not write out anything with no geometry
                    {
                        binaryStream.Write((Int32) product.EntityLabel);
                        binaryStream.Write((UInt16)IfcMetaData.IfcTypeId(product));
                        binaryStream.Write(bb.ToFloatArray());
                        numberOfProducts++;
                    }
                }
            }

            //write out the multiple instances
            
            foreach (var kv in lookup.Where(v=>v.Value>1))
            {
                IXbimShapeGeometryData geometry = ShapeGeometry(kv.Key);
                var instances = ShapeInstancesOf(kv.Key, true).ToList();
                if (instances.Count > 0)
                {
                    numberOfGeometries++;
                    binaryStream.Write(kv.Value); //the number of repetitions of the geometry
                    foreach (IXbimShapeInstanceData xbimShapeInstance in instances)
                        //write out each of the ids style and transforms
                    {
                        binaryStream.Write(xbimShapeInstance.IfcProductLabel);
                        binaryStream.Write((UInt16)xbimShapeInstance.IfcTypeId);
                        binaryStream.Write((UInt32)xbimShapeInstance.InstanceLabel);
                        binaryStream.Write((Int32)xbimShapeInstance.StyleLabel > 0
                            ? xbimShapeInstance.StyleLabel
                            : xbimShapeInstance.IfcTypeId*-1);
                        binaryStream.Write(xbimShapeInstance.Transformation);
                        numberOfTriangles += XbimShapeTriangulation.TriangleCount(geometry.ShapeData);
                        numberOfMatrices++;
                    }
                    numberOfVertices += XbimShapeTriangulation.VerticesCount(geometry.ShapeData);
                   // binaryStream.Write(geometry.ShapeData);
                    var ms = new MemoryStream(geometry.ShapeData);
                    var br = new BinaryReader(ms);
                    var tr = br.ReadShapeTriangulation();
                   
                    tr.Write(binaryStream);
                }
               
            }
            //now do the single instances
            foreach (var kv in lookup.Where(v => v.Value == 1))
            {
                var instances = ShapeInstancesOf(kv.Key, true).ToList();
                if (instances.Count > 0)
                {
                    numberOfGeometries++;
                    IXbimShapeInstanceData xbimShapeInstance = instances[0];

                    IXbimShapeGeometryData geometry = ShapeGeometry(kv.Key);
                    binaryStream.Write(kv.Value); //the number of repetitions of the geometry (1)

                    if (xbimShapeInstance != null)
                    {
                        binaryStream.Write((Int32)xbimShapeInstance.IfcProductLabel);
                        binaryStream.Write((UInt16)xbimShapeInstance.IfcTypeId);
                        binaryStream.Write((Int32)xbimShapeInstance.InstanceLabel);
                        binaryStream.Write((Int32)xbimShapeInstance.StyleLabel > 0
                            ? xbimShapeInstance.StyleLabel
                            : xbimShapeInstance.IfcTypeId*-1);
                    }
                    else
                    {
                        throw new Exception(string.Format("Invalid Shape Instance ID #{0}, did not return any data",
                            kv.Key));
                    }
                    //Read all vertices and normals in the geometry stream and transform
                    var ms = new MemoryStream(geometry.ShapeData);
                    var br = new BinaryReader(ms);
                    var tr = br.ReadShapeTriangulation();
                    var trTransformed = tr.Transform(((XbimShapeInstance) xbimShapeInstance).Transformation);
                    trTransformed.Write(binaryStream);
                    numberOfTriangles += XbimShapeTriangulation.TriangleCount(geometry.ShapeData);
                    numberOfVertices += XbimShapeTriangulation.VerticesCount(geometry.ShapeData);
                }
            }
            binaryStream.Seek(start, SeekOrigin.Begin);
            binaryStream.Write((Int32)numberOfGeometries);
            binaryStream.Write((Int32)numberOfVertices);
            binaryStream.Write((Int32)numberOfTriangles);
            binaryStream.Write((Int32)numberOfMatrices);
            binaryStream.Write((Int32)numberOfProducts);
            binaryStream.Seek(0, SeekOrigin.End); //go back to end
        // ReSharper restore RedundantCast
        }
    }
   }

    