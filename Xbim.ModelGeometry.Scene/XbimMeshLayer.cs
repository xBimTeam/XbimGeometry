using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.Kernel;
using Xbim.Ifc4.PresentationAppearanceResource;

namespace Xbim.ModelGeometry.Scene
{

    /// <summary>
    /// Provides support for a layer of meshes of the same material, TVISIBLE is the type of the mesh geometry required by the graphics adaptor
    /// </summary>
    public class XbimMeshLayer<TMesh, TMaterial>
        where TMesh : IXbimMeshGeometry3D, new()
        where TMaterial : IXbimRenderMaterial, new()
    {
        string _name;
        readonly XbimMeshLayerCollection<TMesh, TMaterial> _subLayerMap = new XbimMeshLayerCollection<TMesh, TMaterial>();
        readonly XbimColourMap _layerColourMap;
        XbimRect3D _boundingBoxVisible = XbimRect3D.Empty;
        XbimRect3D _boundingBoxHidden = XbimRect3D.Empty;

        public IModel Model { get; set; }


        /// <summary>
        /// Bounding box of visible and invisible elements, aligned to the XYZ axis, containing all points in this mesh
        /// </summary>
        /// <param name="forceRecalculation">if true the bounding box is recalculated, if false and previusly calculated the cached version is returned</param>
        /// <returns></returns>
        public XbimRect3D BoundingBoxTotal(bool forceRecalculation = false)
        {
            var ret = BoundingBoxHidden(forceRecalculation);
            ret.Union(BoundingBoxVisible(forceRecalculation));
            return ret;
        }


        /// <summary>
        /// Bounding box of all visible elements, aligned to the XYZ axis, containing all points in this mesh
        /// </summary>
        /// <param name="forceRecalculation">if true the bounding box is recalculated, if false the previous cached version is returned</param>
        /// <returns></returns>
        public XbimRect3D BoundingBoxVisible(bool forceRecalculation = false)
        {
            if (!forceRecalculation && !_boundingBoxVisible.IsEmpty) 
                return _boundingBoxVisible;
            var first = true;
            foreach (var pos in Visible.Positions)
            {
                if (first)
                {
                    _boundingBoxVisible = new XbimRect3D(pos);
                    first = false;
                }
                else
                    _boundingBoxVisible.Union(pos);

            }
            foreach (var sublayer in _subLayerMap)
            {
                XbimRect3D subBox = sublayer.BoundingBoxVisible(forceRecalculation);
                if (!subBox.IsEmpty)
                    _boundingBoxVisible.Union(subBox);
            }
            return _boundingBoxVisible;
        }

        /// <summary>
        /// Bounding box of all hidden elements, aligned to the XYZ axis, containing all points in this mesh
        /// </summary>
        /// <param name="forceRecalculation">if true the bounding box is recalculated, if false the previous cached version is returned</param>
        /// <returns></returns>
        public XbimRect3D BoundingBoxHidden(bool forceRecalculation = false)
        {
            if (!forceRecalculation && !_boundingBoxHidden.IsEmpty) 
                return _boundingBoxHidden;
            var first = true;
            foreach (var pos in Hidden.Positions)
            {
                if (first)
                {
                    _boundingBoxHidden = new XbimRect3D(pos);
                    first = false;
                }
                else
                    _boundingBoxHidden.Union(pos);

            }
            foreach (var sublayer in _subLayerMap)
            {
                XbimRect3D subBox = sublayer.BoundingBoxHidden(forceRecalculation);
                if (!subBox.IsEmpty)
                    _boundingBoxHidden.Union(subBox);
            }
            return _boundingBoxHidden; 
        }

        /// <summary>
        /// The colour map for this scene
        /// </summary>
        public XbimColourMap LayerColourMap
        {
            get { return _layerColourMap; }
        }

        public XbimTexture Style { get; set; }
        
        /// <summary>
        /// A mesh that are currently rendered typically on the graphics adaptor
        /// </summary>
        
        public IXbimMeshGeometry3D Visible = new TMesh();

        /// <summary>
        /// The native graphic engine render material
        /// </summary>
        
        public IXbimRenderMaterial Material = new TMaterial();
        /// <summary>
        /// A mesh that is loaded but not visible on the graphics display
        /// </summary>
      //  public IXbimMeshGeometry3D Hidden = new TVISIBLE();
        public IXbimMeshGeometry3D Hidden = new XbimMeshGeometry3D();
      
        public string Name
        {
            get { return _name; }
            set { _name = value; }
        }

        /// <summary>
        /// Creates a mesh using the default colour (typically white)
        /// </summary>
        public XbimMeshLayer(IModel m)
            :this(m, XbimColour.DefaultColour)
        {
           
        }

        /// <summary>
        /// Create a new layer that will display meshes in the specified colour
        /// If the mesh geometry item has a style specified in the IFC definition sub layers will be created for each style
        /// </summary>
        /// <param name="m">a Model</param>
        /// <param name="colour"></param>
        public XbimMeshLayer(IModel m, XbimColour colour)
        {
            Model = m;
            Style =  XbimTexture.Create(colour);
        }

        public XbimMeshLayer(IModel m, XbimColour colour, XbimColourMap subCategoryColourMap)
            :this(m, colour)
        {
            _layerColourMap = subCategoryColourMap;
        }

        public XbimMeshLayer(IModel m, IIfcSurfaceStyle style)
        {
            Model = m;
            Style =  XbimTexture.Create(style);
           
        }
        public XbimMeshLayer(XbimTexture xbimTexture)
        {
            Style = xbimTexture;
        }

        public XbimMeshLayer(IModel m, XbimTexture xbimTexture)
        {
            Model = m;
            Style = xbimTexture;
        }

        public XbimMeshLayer(XbimColour colour)
        {
            Style = XbimTexture.Create(colour);
        }

        public static implicit operator TMesh(XbimMeshLayer<TMesh, TMaterial> layer)
        {
            return (TMesh)layer.Visible;
        }
        

        /// <summary>
        /// Returns true if the layer has any geometric content
        /// </summary>
        public bool HasContent
        {
            get
            {
                return Hidden.Meshes.Any() || Visible.Meshes.Any();
            }

        }

        /// <summary>
        /// Moves all items in the hidden mesh to the visible mesh
        /// </summary>
        public void ShowAll()
        {
            Hidden.MoveTo(Visible);
            foreach (var subLayer in _subLayerMap)
            {
                subLayer.ShowAll();
            }
        }

        /// <summary>
        /// Moves all items in the visible mesh to the hidden mesh
        /// </summary>
        public void HideAll()
        {
            Visible.MoveTo(Hidden);
            foreach (var subLayer in _subLayerMap)
            {
                subLayer.HideAll();
            }
        }

        /// <summary>
        /// Adds the geometry to the mesh for the given product, returns the mesh fragment details
        /// </summary>
        /// <param name="geometryModel">Geometry to add</param>
        /// <param name="product">The product the geometry represents (this may be a partial representation)</param>
        /// <param name="transform">Transform the geometry to a new location or rotation</param>
        /// <param name="deflection">Deflection for triangulating curves, if null default defelction for the model is used</param>
        /// <param name="modelId">An optional model ID</param>
        public XbimMeshFragment Add(IXbimGeometryModel geometryModel, IfcProduct product, XbimMatrix3D transform, 
            double? deflection = null, short modelId=0)
        {
            return Hidden.Add(geometryModel, product, transform, deflection, modelId);
        }

        /// <summary>
        /// Adds the geometry fragment to the hidden mesh, if the model is not null 
        /// the fragment is placed on a sub layer of the correct style
        /// the sub layer is automaticaly created if it does not exist.
        /// </summary>
        /// <param name="geomData"></param>
        /// <param name="model"></param>
        /// <param name="modelId"></param>
        public void AddToHidden(XbimGeometryData geomData, IModel model, short modelId)
        {
            var addingSuccessfull = Hidden.Add(geomData, modelId); // this is where the geometry is added to the main layer.
            if (addingSuccessfull)
                return;
            //if the main layer is too big split it.
            //try and find a sublayer that is a split of this, i.e. has the same texture
            foreach (var sublayer in _subLayerMap.Reverse())
            {
                if (!Equals(sublayer.Style, Style))
                    continue;
                sublayer.AddToHidden(geomData, model, modelId);//try and add the data to this mesh
                return; //succeeded so return
            }
            //didn't find a layer to add it to so create a new one
            var subLayer = new XbimMeshLayer<TMesh, TMaterial>(model, Style)
            {
                Name = Name + "-" + _subLayerMap.Count
            };
            _subLayerMap.Add(subLayer);
            subLayer.Hidden.Add(geomData, modelId); //this should always pass as it is a new mesh and ifc geom rarely exceeds max mesh size, graphics cards will truncate anyway
        }

        /// <summary>
        /// Adds the geometry fragment to the hidden mesh, if the model is not null 
        /// the fragment is placed on a sub layer of the correct style
        /// the sub layer is automaticaly created if it does not exist.
        /// </summary>
        /// <param name="geomData"></param>
        /// <param name="model"></param>
        public void AddToHidden(XbimGeometryData geomData, IModel model = null)
        {
            int modelId = 0;
            if (model != null)
                modelId = model.UserDefinedId;
            if (model != null && geomData.StyleLabel > 0) // check if we need to put this item on a sub layer
            {
                XbimMeshLayer<TMesh, TMaterial> subLayer;
                var layerName = geomData.StyleLabel.ToString();
                if (!_subLayerMap.Contains(layerName))
                {
                    var style = model.Instances[geomData.StyleLabel] as IfcSurfaceStyle;
                    //create a sub layer
                    subLayer = new XbimMeshLayer<TMesh, TMaterial>(model, style) {Name = layerName};
                    _subLayerMap.Add(subLayer);
                }
                else
                    subLayer = _subLayerMap[layerName];
                
                subLayer.AddToHidden(geomData, null, (short) modelId);
            }
            else
            {
                AddToHidden(geomData, model, (short)modelId);
            }
        }

        public XbimMeshLayerCollection<TMesh, TMaterial> SubLayers 
        {
            get { return _subLayerMap; }
        }

        public void Show()
        {
            if (!Material.IsCreated) Material.CreateMaterial(Style);
            Hidden.MoveTo(Visible);
        }


        public class MeshInfo
        {
            private XbimMeshFragment _mf;
            private readonly XbimMeshLayer<TMesh, TMaterial> _xbimMeshLayer;
            
            public MeshInfo(XbimMeshFragment mf, XbimMeshLayer<TMesh, TMaterial> xbimMeshLayer)
            {
                _mf = mf;
                _xbimMeshLayer = xbimMeshLayer;
            }

            public override string ToString()
            {
                return string.Format("Layer: {0} ({3}) fragment position: {1} lenght: {2}", _xbimMeshLayer._name, _mf.StartPosition, _mf.PositionCount, _xbimMeshLayer.Material.Description);
            }
        }

        public IEnumerable<MeshInfo> GetMeshInfo(int entityLabel)
        {
            foreach (var mf in Visible.Meshes.Where(m => m.EntityLabel == entityLabel))
                yield return new MeshInfo(mf, this);
            foreach (var mf in Hidden.Meshes.Where(m => m.EntityLabel == entityLabel))
                yield return new MeshInfo(mf, this);
            foreach (var layer in SubLayers)
                foreach (var item in layer.GetMeshInfo(entityLabel))
                {
                    yield return item;    
                }
        }

       /// <summary>
        ///  Returns a collection of fragments for this layer, does not traverse sub layers or hidden layers unless arguments are true
       /// </summary>
       /// <param name="entityLabel">the ifc entity label</param>
       /// <param name="includeHidden">Include fragments in hidden layers</param>
       /// <param name="includSublayers">Recurse into sub layers</param>
       /// <returns></returns>
        public IEnumerable<XbimMeshFragment> GetMeshFragments(int entityLabel, bool includeHidden = false, bool includSublayers = false)
        {
            foreach (var mf in Visible.Meshes.Where(m => m.EntityLabel == entityLabel))
                yield return mf;
            if (includeHidden)
                foreach (var mf in Hidden.Meshes.Where(m => m.EntityLabel == entityLabel))
                    yield return mf;
            if (includSublayers)
                foreach (var layer in SubLayers)
                    foreach (var mf in layer.GetMeshFragments(entityLabel, includeHidden, true))
                        yield return mf;
        }

        

        public bool HasEntity(int entityLabel, bool includeHidden = false, bool includSublayers = false)
        {
            return GetMeshFragments(entityLabel, includeHidden, includSublayers).Any();
        }

        public IXbimMeshGeometry3D GetVisibleMeshGeometry3D(int entityLabel, short modelId)
        {
            var fragments = GetMeshFragments(entityLabel).ToArray(); //get all the fragments for this entity in the visible layer
            var maxSize = fragments.Sum(f => f.PositionCount);
            var geometry = new XbimMeshGeometry3D(maxSize);
            foreach (var fragment in fragments)
            {
                var geom = Visible.GetMeshGeometry3D(fragment);
                geometry.Add(geom, fragment.EntityLabel, fragment.EntityType, modelId);
            } 
            return geometry;
        }
        /// <summary>
        /// Returns all the layers including sub layers of this layer
        /// </summary>
        public IEnumerable<XbimMeshLayer<TMesh, TMaterial>> Layers
        {
            get
            {
                foreach (var layer in SubLayers)
                {
                    yield return layer;
                    foreach (var subLayer in layer.Layers)
                    {
                        yield return subLayer;
                    }
                }

            }
        }
        /// <summary>
        /// Resizes the layers so that none has more than USHORT number of indices
        /// </summary>
        public void Balance()
        {
            if (Hidden.TriangleIndices.Count >= ushort.MaxValue) //split the layer
            {
                // todo: needs implementation
                Debug.WriteLine("Too big");
            }
            foreach (var layer in SubLayers)
            {
                layer.Balance();
            }
        }

        /// <summary>
        /// Useful for analysis and debugging purposes (invoked by Querying interface)
        /// </summary>
        /// <param name="indentation">the number of indentation spaces at this tree level</param>
        /// <returns>Enumerable strings of indented elements</returns>
        public IEnumerable<string> LayersTree(int indentation)
        {
            var s = new string(' ', indentation);
            yield return s + _name;
            foreach (var layer in SubLayers)
            {
                foreach (var item in layer.LayersTree(indentation + 1))
                {
                    yield return item;
                } 
            }
        }

        public void Add(string mesh, Type productType, int productLabel, int geometryLabel, XbimMatrix3D? transform = null, short modelId=0)
        {
            Hidden.Add(mesh, productType, productLabel, geometryLabel, transform, modelId);
        }
    }
}
