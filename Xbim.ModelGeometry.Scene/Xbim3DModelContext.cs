#region Directives

using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Net.Configuration;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Isam.Esent.Interop;
using Newtonsoft.Json;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
using Xbim.Common.XbimExtensions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.Interfaces;
using Xbim.IO;
using Xbim.ModelGeometry.Scene.Clustering;
using Xbim.ModelGeometry.Scene.Extensions;


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
            private readonly List<XbimShapeInstance> _argumentIds;
            private readonly List<XbimShapeInstance> _cutToolIds;
            private readonly List<XbimShapeInstance> _projectToolIds;

            public XbimBooleanDefinition(IList<XbimShapeInstance> argumentIds, IList<XbimShapeInstance> cutToolIds, IList<XbimShapeInstance> projectToolIds, int context, int styleId)
            {
                if (argumentIds.Any()) _argumentIds = new List<XbimShapeInstance>(argumentIds);
                if (cutToolIds.Any()) _cutToolIds = new List<XbimShapeInstance>(cutToolIds);
                if (projectToolIds.Any()) _projectToolIds = new List<XbimShapeInstance>(projectToolIds);
                _contextId = context;
                _styleId = styleId;
            }
            /// <summary>
            /// The ids of the geometry arguments the tools will be applied to 
            /// </summary>
            public IList<XbimShapeInstance> ArgumentIds
            {
                get
                {
                    return _argumentIds ?? EmptyShapeList;
                }              
            }

            public int ContextId
            {
                get
                {
                    return _contextId;
                }
            }

            /// <summary>
            /// The ids of geometry tools to be used in the boolean cut operation
            /// </summary>
            public IEnumerable<XbimShapeInstance> CutToolIds
            {
                get
                {
                    return _cutToolIds == null ? Enumerable.Empty<XbimShapeInstance>() : _cutToolIds;
                }
            }
            /// <summary>
            /// The ids of geometry tools to be used in the boolean project operation
            /// </summary>
            public IEnumerable<XbimShapeInstance> ProjectToolIds
            {
                get
                {
                    return _projectToolIds == null ? Enumerable.Empty<XbimShapeInstance>() : _projectToolIds;
                }
            }

            public int StyleId
            {
                get
                {
                    return _styleId;
                }          
            }
        }

        //private struct to hold details of references to geometries
        private struct GeometryReference
        {
            public XbimRect3D BoundingBox;
            public int GeometryId;
            public int StyleLabel;
        }

        private class IIfcRepresentationContextCollection : KeyedCollection<int, IIfcRepresentationContext>
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
            private bool _disposed = false;
            bool _adjustWCS;

            public XbimCreateContextHelper(IModel model, IIfcRepresentationContextCollection contexts, bool adjustWCS = true)
            {
                Model = model;
                Contexts = contexts;
                _adjustWCS = adjustWCS;
            }
          
            /// <summary>
            /// The key is the IIfc label of the geometry the value is the database record number
            /// </summary>
            internal ConcurrentDictionary<int,int> GeometryShapeLookup { get; private set; }

            private IModel Model { get; set; }
            private IIfcRepresentationContextCollection Contexts { get; set; }
            internal ParallelOptions ParallelOptions { get; private set; }
            internal XbimPlacementTree PlacementTree { get; private set; }
            internal HashSet<int> MappedShapeIds { get; private set; }
            internal HashSet<int> FeatureElementShapeIds { get; private set; }
            internal List<IGrouping<IIfcElement, IIfcFeatureElement>> OpeningsAndProjections { get; private set; }
            private HashSet<int> VoidedShapeIds { get; set; }
            internal HashSet<int> ProductShapeIds { get; private set; }
            internal ConcurrentDictionary<int, IXbimGeometryObject> CachedGeometries { get; private set; }
            internal int Total { get; private set; }
            internal int PercentageParsed { get; set; }
            internal int Tally { get; set; }
            internal Dictionary<int, int> SurfaceStyles { get; private set; }

            internal Dictionary<IIfcRepresentationContext, ConcurrentQueue<XbimBBoxClusterElement>> Clusters { get; private set;
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
                    PlacementTree = new XbimPlacementTree(Model, _adjustWCS);
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
                            r.RelatingBuildingElement.Representation != null &&
                            r.RelatedOpeningElement.Representation != null).ToList();
                foreach (var openingRelation in openingRelations)
                {
                    // process parts
                    IEnumerable<IIfcObjectDefinition> childrenElements;
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
                var projectingRelations = Model.Instances.OfType<IIfcRelVoidsElement>()
                    .Where(
                        r =>
                            r.RelatingBuildingElement.Representation != null &&
                            r.RelatedOpeningElement.Representation != null).ToList();
                foreach (var projectionRelation in projectingRelations)
                {
                    // process parts
                    IEnumerable<IIfcObjectDefinition> childrenElements;
                    if (compoundElementsDictionary.TryGetValue(projectionRelation.RelatingBuildingElement,
                        out childrenElements))
                    {
                        elementsWithFeatures.AddRange(
                            childrenElements.OfType<IIfcElement>().Select(childElement => new ElementWithFeature()
                            {
                                Element = childElement,
                                Feature = projectionRelation.RelatedOpeningElement
                            }));
                    }

                    // process parent
                    elementsWithFeatures.Add(new ElementWithFeature()
                    {
                        Element = projectionRelation.RelatingBuildingElement,
                        Feature = projectionRelation.RelatedOpeningElement
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
                        styledItemGrouping.SelectMany(st => st.Styles.OfType<IIfcSurfaceStyle>()).FirstOrDefault();
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
                foreach (var product in Model.Instances.OfType<IIfcProduct>(true).Where(p => p.Representation != null))
                {
                    var isFeatureElementShape = product is IIfcFeatureElement ||
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
                            foreach (var shape in rep.Items.Where(i => !(i is IIfcGeometricSet)))
                            {
                                var mappedItem = shape as IIfcMappedItem;
                                if (mappedItem != null)
                                {
                                    MappedShapeIds.Add(mappedItem.EntityLabel);
                                    //make sure any shapes mapped are in the set to process as well
                                    foreach (var item in mappedItem.MappingSource.MappedRepresentation.Items)
                                    {
                                        if (item != null && !(item is IIfcGeometricSet))
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
                if (_disposed) return;
                _disposed = true;
                foreach (var cachedGeom in CachedGeometries)
                {
                    cachedGeom.Value.Dispose();
                }
                GC.SuppressFinalize(this);
            }
        }

        #endregion

        public static readonly ILogger Logger = LoggerFactory.GetLogger();
        private readonly IIfcRepresentationContextCollection _contexts;
        private XbimGeometryEngine _engine;

        private XbimGeometryEngine Engine
        {
            get { return _engine ?? (_engine = new XbimGeometryEngine()); }
        }
        private readonly IModel _model;

        private bool _contextIsPersisted;

        //The maximum extent for any dimension of any products bouding box 
        //private double _maxXyz;
       
        /// <summary>
        ///     Initialises a model context from the model
        /// </summary>
        /// <param name="model"></param>
        /// <param name="contextType"></param>
        /// <param name="contextIdentifier"></param>
        public Xbim3DModelContext(IModel model, string contextType = "model", string contextIdentifier = "body")
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
                    Logger.InfoFormat(
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
                _contexts = new IIfcRepresentationContextCollection();
                //var roundingPrecisions = new Dictionary<IIfcRepresentationContext, int>();
                //var contextLookup = new Dictionary<short, IIfcRepresentationContext>();
                foreach (var context in contexts)
                {
                    _contexts.Add(context);
                    //roundingPrecisions.Add(context,
                    //    Math.Abs(context.DefaultPrecision) < 1e-9
                    //        ? 0
                    //        : Math.Abs((int) Math.Log10(context.DefaultPrecision)));
                    //contextLookup.Add(GetContextId(context), context);
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

        public IModel Model
        {
            get { return _model; }
        }

        //private static short GetContextId(IIfcRepresentationContext context)
        //{
        //    return (short) ((context.EntityLabel >> 16) ^ context.EntityLabel);
        //}

        /// <summary>
        /// </summary>
        /// <param name="progDelegate"></param>
        /// <param name="geomStorageType">The type of geometry storage type to use, typically Polyhedron or  PolyhedronBinary</param>
        /// <returns></returns>
        public bool CreateContext(XbimGeometryType geomStorageType = XbimGeometryType.Polyhedron, ReportProgressDelegate progDelegate = null, bool adjustWcs = true)
        {
            if (_contexts == null || Engine == null) return false;
            if (_contextIsPersisted) return false; //already created it
            var geometryStore = _model.GeometryStore;
            if (geometryStore == null) return false;
            using (var geometryTransaction = geometryStore.BeginInit())
            {
                if (geometryTransaction == null) return false;
                using (var contextHelper = new XbimCreateContextHelper(_model, _contexts, adjustWcs))
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
                                    () => WriteShapeGeometriesToStore(contextHelper, shapeGeometries,geometryTransaction)))
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
                        using (var writeToDb = Task.Factory.StartNew(() => WriteFeatureElementsToStore(features,geometryTransaction)))
                        {
                            try
                            {
                                processed = WriteFeatureElements(contextHelper, features, progDelegate, geomStorageType,geometryTransaction);
                            }
                            finally
                            {
                                //wait for the geometry shapes to be written
                                features.CompleteAdding();
                                writeToDb.Wait();
                            }
                        }
                        if (progDelegate != null) progDelegate(-1, "WriteProductShapes");
                        var productsRemaining = _model.Instances.OfType<IIfcProduct>()
                            .Where(p =>
                                p.Representation != null
                                && !processed.Contains(p.EntityLabel)
                            ).ToList();


                        WriteProductShapes(contextHelper, productsRemaining,geometryTransaction);
                        if (progDelegate != null) progDelegate(101, "WriteProductShapes");
                        //Write out the actual representation item reference count
                        
                        
                        //Write out the regions of the model
                        if (progDelegate != null) progDelegate(-1, "WriteRegionsToDb");
                        foreach (var cluster in contextHelper.Clusters)
                        {
                            WriteRegionsToDb(cluster.Key, cluster.Value, geometryTransaction);
                        }
                        if (progDelegate != null) progDelegate(101, "WriteRegionsToDb");
                        _contextIsPersisted = true;                      
                    }
                }
                geometryStore.EndInit(geometryTransaction);
                return true;
            }
        }



        private void WriteShapeGeometriesToStore(XbimCreateContextHelper contextHelper,
            BlockingCollection<IXbimShapeGeometryData> shapeGeometries, IGeometryStoreInitialiser txn)
        {
            IXbimShapeGeometryData shapeGeom = null;
            while (!shapeGeometries.IsCompleted)
            {
                try
                {
                    if (shapeGeometries.TryTake(out shapeGeom))
                    {
                        var refCounter = new GeometryReference
                        {
                            BoundingBox = ((XbimShapeGeometry) shapeGeom).BoundingBox,
                            GeometryId = txn.AddShapeGeometry(shapeGeom)

                        };
                        int styleLabel;
                        contextHelper.SurfaceStyles.TryGetValue(shapeGeom.IfcShapeLabel, out styleLabel);
                        refCounter.StyleLabel = styleLabel;
                        contextHelper.ShapeLookup.TryAdd(shapeGeom.IfcShapeLabel, refCounter);
                        if (contextHelper.CachedGeometries.ContainsKey(shapeGeom.IfcShapeLabel))
                            //keep a record of the IFC label and database record mapping
                            contextHelper.GeometryShapeLookup.TryAdd(shapeGeom.ShapeLabel,
                                shapeGeom.IfcShapeLabel);

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
            }
        }



        private void WriteFeatureElementsToStore(
            BlockingCollection<Tuple<XbimShapeInstance, IXbimShapeGeometryData>> shapeFeatures, IGeometryStoreInitialiser txn)
        {
            while (!shapeFeatures.IsCompleted)
            {
                try
                {
                    Tuple<XbimShapeInstance, IXbimShapeGeometryData> shapeFeature;
                    if (shapeFeatures.TryTake(out shapeFeature))
                    {
                        var shapeInstance = shapeFeature.Item1;
                        var shapeGeometry = shapeFeature.Item2;
                        shapeInstance.ShapeGeometryLabel = txn.AddShapeGeometry(shapeGeometry);
                        txn.AddShapeInstance(shapeInstance, shapeInstance.ShapeGeometryLabel);
                    }
                }
                catch (InvalidOperationException)
                {
                    break;
                }
            }
        }

        private HashSet<int> WriteFeatureElements(XbimCreateContextHelper contextHelper,
            BlockingCollection<Tuple<XbimShapeInstance, IXbimShapeGeometryData>> features,
            ReportProgressDelegate progDelegate, XbimGeometryType geomType, IGeometryStoreInitialiser txn
            )
        {
            var processed = new HashSet<int>();
            var localPercentageParsed = 0;
            var localTally = 0;
            var featureCount = contextHelper.OpeningsAndProjections.Count;
            if (progDelegate != null) progDelegate(-1, "WriteFeatureElements (" + contextHelper.OpeningsAndProjections.Count + " elements)");

            var dupIds = new HashSet<int>();
            var allIds = new HashSet<int>();
            var booleanOps = new List<XbimBooleanDefinition>();
            double precision = Math.Max(_model.ModelFactors.OneMilliMeter / 50, _model.ModelFactors.Precision); //set the precision to 100th mm but never less than precision

            //   Parallel.ForEach(contextHelper.OpeningsAndProjections, contextHelper.ParallelOptions, pair =>
            //         foreach (IGrouping<IIfcElement, IIfcFeatureElement> pair in contextHelper.OpeningsAndProjections)
            foreach (IGrouping<IIfcElement, IIfcFeatureElement> pair in contextHelper.OpeningsAndProjections)
            {
                int context = 0;
                int styleId = 0;//take the style of any part of the main shape
                var element = pair.Key;

               
                var elementShapes = WriteProductShape(contextHelper, element, false,txn );
                var arguments = new List<XbimShapeInstance>();
                foreach (var elemShape in elementShapes)
                {
                    if (!allIds.Add(elemShape.ShapeGeometryLabel))
                        dupIds.Add(elemShape.ShapeGeometryLabel);
                    arguments.Add(elemShape);
                    context = elemShape.RepresentationContext;
                    if (elemShape.StyleLabel > 0) styleId = elemShape.StyleLabel;
                }

                if (arguments.Count == 0)
                {
                    //Logger.WarnFormat(
                    //    "WM003: {2}[#{0}]-{1} does not have a solid representation, openings cannot be formed",
                    //    element.EntityLabel, element.Name, element.GetType().Name);
                    // the warning is obsolete because it's been handled int the original geometry creation
                    processed.Add(element.EntityLabel);
                    continue;
                }
                var cutTools = new List<XbimShapeInstance>();
                var projectTools = new List<XbimShapeInstance>();


                foreach (var feature in pair)
                {
                    var isCut = feature is IIfcFeatureElementSubtraction;

                    var featureShapes = WriteProductShape(contextHelper, feature, false, txn);
                    foreach (var featureShape in featureShapes)
                    {
                        if (!allIds.Add(featureShape.ShapeGeometryLabel))
                            dupIds.Add(featureShape.ShapeGeometryLabel);
                        if (isCut)
                            cutTools.Add(featureShape);
                        else
                            projectTools.Add(featureShape);
                    }
                    processed.Add(feature.EntityLabel);

                }
                var boolOp = new XbimBooleanDefinition(arguments, cutTools, projectTools, context, styleId);
                booleanOps.Add(boolOp);
            }

            Parallel.ForEach(booleanOps, contextHelper.ParallelOptions, bop =>
            //         foreach (IGrouping<IIfcElement, IIfcFeatureElement> pair in contextHelper.OpeningsAndProjections)
            {
                Interlocked.Increment(ref localTally);
                int elementLabel = 0;
                short typeId = 0;
                try
                {
                    if (bop.ArgumentIds.Any())
                    {
                        elementLabel = bop.ArgumentIds.First().IfcProductLabel;
                        typeId = bop.ArgumentIds.First().IfcTypeId;
                        //Get all the parts of this element into a set of solid geometries
                        var elementGeom = Engine.CreateGeometryObjectSet();

                        foreach (var argument in bop.ArgumentIds)
                        {
                            var geom = contextHelper.GetGeometryFromCache(argument, dupIds.Contains(argument.ShapeGeometryLabel));
                            if (geom != null)
                                elementGeom.Add(geom);
                            else
                                Logger.WarnFormat(
                               "WM013: {0}[#{1}] is an element that has some 3D geometric form definition missing",
                               _model.Metadata.ExpressType(argument.IfcTypeId).Name, argument.IfcProductLabel);

                        }

                        //now build the openings
                        var allOpenings = Engine.CreateSolidSet();
                        foreach (var openingShape in bop.CutToolIds)
                        {
                            var openingGeom = contextHelper.GetGeometryFromCache(openingShape, dupIds.Contains(openingShape.ShapeGeometryLabel));
                            if (openingGeom != null)
                                allOpenings.Add(openingGeom);
                            else
                                Logger.WarnFormat(
                               "WM014: {0}[#{1}] is an opening that has some 3D geometric form definition missing",
                               _model.Metadata.ExpressType(openingShape.IfcTypeId).Name, openingShape.IfcProductLabel);
                        }

                        //now all the projections
                        var allProjections = Engine.CreateSolidSet();


                        foreach (var projectionShape in bop.ProjectToolIds)
                        {
                            var projGeom = contextHelper.GetGeometryFromCache(projectionShape, dupIds.Contains(projectionShape.ShapeGeometryLabel));
                            if (projGeom != null)
                                allProjections.Add(projGeom);
                            else
                                Logger.WarnFormat(
                              "WM005: {0}[#{1}] is an projection that has no 3D geometric form definition",
                              _model.Metadata.ExpressType(projectionShape.IfcTypeId).Name, projectionShape.IfcProductLabel);
                        }

                        //make the finished shape
                        if (allProjections.Any())
                        {
                            var nextGeom = elementGeom.Union(allProjections, precision);
                            if (nextGeom.IsValid)
                            {
                                if (nextGeom.First != null && nextGeom.First.IsValid)
                                    elementGeom = nextGeom;
                                else
                                    Logger.WarnFormat(
                               "WM015: Joining of projections in {1}[#{0}] has resulted in an empty shape",
                               elementLabel, _model.Metadata.ExpressType(typeId).Name);
                            }
                            else
                                Logger.WarnFormat(
                               "WM016: Joining of projections in {1}[#{0}] has failed, projections have been ignored",
                               elementLabel, _model.Metadata.ExpressType(typeId).Name);
                        }


                        if (allOpenings.Any())
                        {

                            var nextGeom = elementGeom.Cut(allOpenings, precision);
                            if (nextGeom.IsValid)
                            {
                                if (nextGeom.First != null && nextGeom.First.IsValid)
                                    elementGeom = nextGeom;
                                else
                                    Logger.WarnFormat(
                               "WM009: Cutting of openings in {1}[#{0}] has resulted in an empty shape",
                               elementLabel, _model.Metadata.ExpressType(typeId).Name);
                            }
                            else
                                Logger.WarnFormat(
                               "WM008: Cutting of openings in {1}[#{0}] has failed, openings have been ignored",
                               elementLabel, _model.Metadata.ExpressType(typeId).Name);
                        }

                        ////now add to the DB               
                        IModelFactors mf = _model.ModelFactors;
                        foreach (var geom in elementGeom)
                        {
                            IXbimShapeGeometryData shapeGeometry = new XbimShapeGeometry
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
                            shapeGeometry.ShapeData = memStream.ToArray();
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
                                    IfcTypeId = typeId,
                                    Transformation = XbimMatrix3D.Identity,
                                    BoundingBox = elementGeom.BoundingBox
                                };
                                features.Add(new Tuple<XbimShapeInstance, IXbimShapeGeometryData>(shapeInstance,
                                    shapeGeometry));
                            }
                        }
                        processed.Add(elementLabel);
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
                    Logger.WarnFormat(
                        "WM007: {0}[#{1}] - contains openings but  its geometry can not be built, {2}",
                       _model.Metadata.ExpressType(typeId).Name, elementLabel, e.Message);
                }
                //if (progDelegate != null) progDelegate(101, "FeatureElement, (#" + element.EntityLabel + " ended)");
            }
            );
            contextHelper.PercentageParsed = localPercentageParsed;
            contextHelper.Tally = localTally;
            if (progDelegate != null) progDelegate(101, "WriteFeatureElements, (" + localTally + " written)");
            return processed;
        }

        private void WriteProductShapes(XbimCreateContextHelper contextHelper, IEnumerable<IIfcProduct> products, IGeometryStoreInitialiser txn)
        {
            var localTally = contextHelper.Tally;
            var localPercentageParsed = contextHelper.PercentageParsed;

            Parallel.ForEach(products, contextHelper.ParallelOptions, product =>
                //    foreach (var product in products)
                {
                    //select representations that are in the required context
                    //only want solid representations for this context, but rep type is optional so just filter identified 2d elements
                    //we can only handle one representation in a context and this is in an implementers agreement
                    var rep =
                        product.Representation.Representations.FirstOrDefault(r => _contexts.Contains(r.ContextOfItems) &&
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
                .FirstOrDefault(r => _contexts.Contains(r.ContextOfItems)
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
            var placementTransform = contextHelper.PlacementTree[element.ObjectPlacement.EntityLabel];
            var contextId = rep.ContextOfItems.EntityLabel;

            //write out any shapes it has                   
            foreach (var shape in rep.Items)
            {
                if (shape is IIfcMappedItem)
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
                        else if (!(mapShape is IIfcGeometricSet)) //ignore non solid geometry sets
                            Logger.WarnFormat("Failed to find shape #{0}", mapShape.EntityLabel);
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
            
            //var mapLookup = new ConcurrentDictionary<int, int>();
            if (progDelegate != null) progDelegate(-1, "WriteShapeGeometries (" + contextHelper.ProductShapeIds.Count + " shapes)");
            var precision = Model.ModelFactors.Precision;
            var deflection = Model.ModelFactors.DeflectionTolerance;
            var deflectionAngle = Model.ModelFactors.DeflectionAngle;
           
           Parallel.ForEach(contextHelper.ProductShapeIds, contextHelper.ParallelOptions, shapeId =>
        //  foreach (var shapeId in contextHelper.ProductShapeIds)
            {
                Interlocked.Increment(ref localTally);
                var shape = (IIfcGeometricRepresentationItem)Model.Instances[shapeId];
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
                    // Console.WriteLine(((IIfcFacetedBrep) shape).Outer.CfsFaces.Count);
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

                                geomModel = Engine.Create(shape);
                                if (geomModel != null && geomModel.IsValid)
                                {
                                    shapeGeom = Engine.CreateShapeGeometry(geomModel, precision, deflection,
                                        deflectionAngle, geomStorageType);
                                    if (isFeatureElementShape)
                                    //we need for boolean operations later, add the polyhedron if the face is planar
                                    contextHelper.CachedGeometries.TryAdd(shapeId, geomModel);
                                }
                            
                            }

                            if (shapeGeom == null || shapeGeom.ShapeData == null || shapeGeom.ShapeData.Length == 0)
                                Logger.InfoFormat("WM001: {1}[#{0}] is an empty shape", shapeId,
                                    shape.GetType().Name);
                            else
                            {
                                shapeGeom.IfcShapeLabel = shapeId;
                                shapeGeometries.Add(shapeGeom);
                            }
                            if (geomModel != null && geomModel.IsValid && !isFeatureElementShape)
                            {
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


        private void WriteRegionsToDb(IIfcRepresentationContext context, IEnumerable<XbimBBoxClusterElement> elementsToCluster, IGeometryStoreInitialiser txn)
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
        private XbimShapeInstance WriteShapeInstanceToStore( int shapeLabel, int styleLabel, int ctxtId,
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
                    Logger.ErrorFormat("Failed to add product geometry for entity #{0}, reason {1}", product.EntityLabel,
                        e.Message);
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
            int openingLabel = _model.Metadata.ExpressTypeId(typeof(IIfcOpeningElement));
            int projectionLabel = _model.Metadata.ExpressTypeId(typeof(IIfcProjectionElement));

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

                if (
                    (incorporateFeatures && instance.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded)
                    ||
                    (!incorporateFeatures && instance.RepresentationType == XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded)
                   )
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

        /// <summary>
        ///     Returns an enumerable of all the unique surface styles used in this context, optionally pass in a colour map to
        ///     obtain styles defaulted by type where they have no definition in the model
        /// </summary>
        /// <returns></returns>
        public IEnumerable<XbimTexture> SurfaceStyles(XbimColourMap colourMap = null)
        {
            if (colourMap == null) colourMap = new XbimColourMap();
            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var context in _contexts)
                {
                    foreach (var surfaceStyle in reader.StyleIds(context.EntityLabel))
                    {
                        if (surfaceStyle > 0) //we have a surface style
                        {
                            var ss = (IIfcSurfaceStyle)_model.Instances[surfaceStyle];
                            yield return new XbimTexture().CreateTexture(ss);                           
                        }
                        else
                        {
                            var theType = _model.Metadata.GetType((short)Math.Abs(surfaceStyle));
                            var texture = new XbimTexture().CreateTexture(colourMap[theType.Name]);
                            //get the colour to use
                            texture.DefinedObjectId = surfaceStyle;
                            yield return texture;
                        }
                    }
                }
            }        
        }

        public IEnumerable<XbimRegion> GetRegions()
        {
            var contextIds = _contexts.Select(c => c.EntityLabel).ToList();
            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var regions in reader.Regions)
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
                    foreach (var shapeInstance in reader.ShapeInstancesOfGeometry(geometry.ShapeLabel, context.EntityLabel))
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
                        shapeInstance.RepresentationType == (byte) XbimGeometryRepresentationType.OpeningsAndAdditionsOnly) // then look for feature shape
                    ||
                    // normal items
                    (
                        (ignoreFeatures && shapeInstance.RepresentationType == (byte) XbimGeometryRepresentationType.OpeningsAndAdditionsExcluded)
                        ||
                        shapeInstance.RepresentationType == (byte) XbimGeometryRepresentationType.OpeningsAndAdditionsIncluded
                    )
                );
        }

        public IEnumerable<XbimShapeInstance> ShapeInstancesOf(int geometryLabel, bool ignoreFeatures = false)
        {
            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var context in _contexts)
                {
                    foreach (var shapeInstance in reader.ShapeInstancesOfGeometry(geometryLabel, context.EntityLabel))
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
                    foreach (var shapeInstance in reader.ShapeInstancesOfEntity(product, context.EntityLabel))
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
        ///     Returns the shape instances that have a surface style of the  specified texture in this context
        /// </summary>
        /// <returns></returns>
        public IEnumerable<XbimShapeInstance> ShapeInstancesOf(XbimTexture texture)
        {
            using (var reader = _model.GeometryStore.BeginRead())
            {
                foreach (var context in _contexts)
                {
                    if (texture.DefinedObjectId > 0)
                    {
                        foreach (var shapeInstance in reader.ShapeInstancesOfStyle(texture.DefinedObjectId, context.EntityLabel))
                        {
                            if (context.EntityLabel == shapeInstance.RepresentationContext)
                            {
                                yield return shapeInstance;
                            }
                        }
                    }
                    else
                    {
                        foreach (var shapeInstance in reader.ShapeInstancesOfEntityType(texture.DefinedObjectId, context.EntityLabel))
                        {
                            if (context.EntityLabel == shapeInstance.RepresentationContext && shapeInstance.StyleLabel < 0)
                            {
                                yield return shapeInstance;
                            }
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
                shapeInstance.InstanceLabel, shapeInstance.Transformation,(short) _model.UserDefinedId);
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

      
    }
   }

    