using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;
using Xbim.Ifc2x3.Kernel;
using Xbim.Ifc2x3.PresentationAppearanceResource;
using Xbim.Ifc2x3.PresentationResource;
using Xbim.IO;
using Xbim.XbimExtensions;

namespace Xbim.ModelGeometry.Scene
{

    /// <summary>
    /// Provides support for a layer of meshes of the same material, TVISIBLE is the type of the mesh geometry required by the graphics adaptor
    /// </summary>
    public class XbimMeshLayer<TVISIBLE, TMATERIAL>
        where TVISIBLE : IXbimMeshGeometry3D, new()
        where TMATERIAL : IXbimRenderMaterial, new()
    {
        string name;
        XbimMeshLayerCollection<TVISIBLE, TMATERIAL> subLayerMap = new XbimMeshLayerCollection<TVISIBLE, TMATERIAL>();
        XbimColourMap layerColourMap;
        XbimRect3D boundingBoxVisible = XbimRect3D.Empty;
        XbimRect3D boundingBoxHidden = XbimRect3D.Empty;
        private XbimModel model;

        public XbimModel Model
        {
            get { return model; }
            set { model = value; }
        }
        /// <summary>
        /// Bounding box of all visible elements, aligned to the XYZ axis, containing all points in this mesh
        /// </summary>
        /// <param name="forceRecalculation">if true the bounding box is recalculated, if false the previous cached version is returned</param>
        /// <returns></returns>
        public XbimRect3D BoundingBoxVisible(bool forceRecalculation = false)
        {
            if (forceRecalculation || boundingBoxVisible.IsEmpty)
            {
                bool first = true;
                foreach (var pos in Hidden.Positions)
                {
                    if (first)
                    {
                        boundingBoxVisible = new XbimRect3D(pos);
                        first = false;
                    }
                    else
                        boundingBoxVisible.Union(pos);

                }
                foreach (var sublayer in subLayerMap)
                {
                    XbimRect3D subBox = sublayer.BoundingBoxVisible(forceRecalculation);
                    if (!subBox.IsEmpty)
                        boundingBoxVisible.Union(subBox);
                }
            }
            return boundingBoxVisible; 
        }

        /// <summary>
        /// Bounding box of all hidden elements, aligned to the XYZ axis, containing all points in this mesh
        /// </summary>
        /// <param name="forceRecalculation">if true the bounding box is recalculated, if false the previous cached version is returned</param>
        /// <returns></returns>
        public XbimRect3D BoundingBoxHidden(bool forceRecalculation = false)
        {
            if (forceRecalculation || boundingBoxHidden.IsEmpty)
            {
                bool first = true;
                foreach (var pos in Hidden.Positions)
                {
                    if (first)
                    {
                        boundingBoxHidden = new XbimRect3D(pos);
                        first = false;
                    }
                    else
                        boundingBoxHidden.Union(pos);

                }
                foreach (var sublayer in subLayerMap)
                {
                    XbimRect3D subBox = sublayer.BoundingBoxHidden(forceRecalculation);
                    if (!subBox.IsEmpty)
                        boundingBoxHidden.Union(subBox);
                }
            }
            return boundingBoxHidden; 
        }

        /// <summary>
        /// The colour map for this scene
        /// </summary>
        public XbimColourMap LayerColourMap
        {
            get { return layerColourMap; }
        }

        public XbimTexture Style { get; set; }
        
        /// <summary>
        /// A mesh that are currently rendered typically on the graphics adaptor
        /// </summary>
        
        public IXbimMeshGeometry3D Visible = new TVISIBLE();

        /// <summary>
        /// The native graphic engine render material
        /// </summary>
        
        public IXbimRenderMaterial Material = new TMATERIAL();
        /// <summary>
        /// A mesh that is loaded but not visible on the graphics display
        /// </summary>
      //  public IXbimMeshGeometry3D Hidden = new TVISIBLE();
        public IXbimMeshGeometry3D Hidden = new XbimMeshGeometry3D();
      
        public string Name
        {
            get { return name; }
            set { name = value; }
        }

        /// <summary>
        /// Creates a mesh using the default colour (typically white)
        /// </summary>
        public XbimMeshLayer(XbimModel m)
            :this(m, XbimColour.Default)
        {
           
        }

        /// <summary>
        /// Create a new layer that will display meshes in the specified colour
        /// If the mesh geometry item has a style specified in the IFC definition sub layers will be created for each style
        /// </summary>
        /// <param name="colour"></param>
        public XbimMeshLayer(XbimModel m, XbimColour colour)
        {
            model = m;
            Style = new XbimTexture().CreateTexture(colour);
        }

        public XbimMeshLayer(XbimModel m, XbimColour colour, XbimColourMap subCategoryColourMap)
            :this(m, colour)
        {
            layerColourMap = subCategoryColourMap;
        }

        public XbimMeshLayer(XbimModel m, IfcSurfaceStyle style)
        {
            model = m;
            Style = new XbimTexture().CreateTexture(style);
           
        }
        public XbimMeshLayer(XbimTexture xbimTexture)
        {
            Style = xbimTexture;
        }

        public XbimMeshLayer(XbimModel m, XbimTexture xbimTexture)
        {
            model = m;
            Style = xbimTexture;
        }

        public XbimMeshLayer(XbimColour colour)
        {
            Style = new XbimTexture().CreateTexture(colour);
        }

        public static implicit operator TVISIBLE(XbimMeshLayer<TVISIBLE, TMATERIAL> layer)
        {
            return (TVISIBLE)layer.Visible;
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
            foreach (var subLayer in subLayerMap)
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
            foreach (var subLayer in subLayerMap)
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
        public XbimMeshFragment Add(IXbimGeometryModel geometryModel, IfcProduct product, XbimMatrix3D transform, double? deflection = null, short modelId=0)
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
        public void AddToHidden(XbimGeometryData geomData, XbimModel model = null)
        {
            if (model != null && geomData.StyleLabel > 0) //check if we need to put this item on a sub layer
            {
                XbimMeshLayer<TVISIBLE, TMATERIAL> subLayer;
                string layerName = geomData.StyleLabel.ToString();
                if (!subLayerMap.Contains(layerName))
                {
                    IfcSurfaceStyle style = model.InstancesLocal[geomData.StyleLabel] as IfcSurfaceStyle;
                    //create a sub layer
                    subLayer = new XbimMeshLayer<TVISIBLE, TMATERIAL>(model,style);
                    subLayer.Name = layerName;
                    subLayerMap.Add(subLayer);
                }
                else
                    subLayer = subLayerMap[layerName];
                
                subLayer.AddToHidden(geomData);
            }
            else
            {
                bool AddingSuccessfull = Hidden.Add(geomData); // this is where the geometry is added to the main layer.
                if (!AddingSuccessfull) 
                {
                    //if the main layer is too big split it.
                    //try and find a sublayer that is a split of this, i.e. has the same texture
                    foreach (var sublayer in subLayerMap.Reverse())
                    {
                        if (sublayer.Style == this.Style) //FOUND THE LAST ONE WITH THE SAME STYLE
                        {
                            sublayer.AddToHidden(geomData);//try and add the data to this mesh
                            return; //succeeded so return
                        }
                    }
                    //didn't find a layer to add it to so create a new one
                    XbimMeshLayer<TVISIBLE, TMATERIAL> subLayer = new XbimMeshLayer<TVISIBLE, TMATERIAL>(model, this.Style);
                    subLayer.Name = this.Name + "-" + subLayerMap.Count;
                    subLayerMap.Add(subLayer);
                    subLayer.Hidden.Add(geomData); //this should always pass as it is a new mesh and ifc geom rarely exceeds max mesh size, graphics cards will truncate anyway
                }
            }
        }

       
        public XbimMeshLayerCollection<TVISIBLE, TMATERIAL> SubLayers 
        {
            get
            {
                return subLayerMap;
            }
        }

        public void Show()
        {
            if (!Material.IsCreated) Material.CreateMaterial(Style);
            Hidden.MoveTo(Visible);
        }


        public class MeshInfo
        {
            private XbimMeshFragment mf;
            private XbimMeshLayer<TVISIBLE, TMATERIAL> xbimMeshLayer;
            
            public MeshInfo(XbimMeshFragment mf, XbimMeshLayer<TVISIBLE, TMATERIAL> xbimMeshLayer)
            {
                // TODO: Complete member initialization
                this.mf = mf;
                this.xbimMeshLayer = xbimMeshLayer;
            }

            public override string ToString()
            {
                return string.Format("Layer: {0} ({3}) fragment position: {1} lenght: {2}", xbimMeshLayer.name, mf.StartPosition, mf.PositionCount, xbimMeshLayer.Material.Description);
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
        internal IEnumerable<XbimMeshFragment> GetMeshFragments(int entityLabel, bool includeHidden = false, bool includSublayers = false)
        {
            foreach (var mf in Visible.Meshes.Where(m => m.EntityLabel == entityLabel))
                yield return mf;
            if (includeHidden)
                foreach (var mf in Hidden.Meshes.Where(m => m.EntityLabel == entityLabel))
                    yield return mf;
            if (includSublayers)
                foreach (var layer in SubLayers)
                    foreach (var mf in layer.GetMeshFragments(entityLabel, includeHidden, includSublayers))
                        yield return mf;
        }

        

        public bool HasEntity(int entityLabel, bool includeHidden = false, bool includSublayers = false)
        {
            foreach (var item in  this.GetMeshFragments(entityLabel, includeHidden, includSublayers))
	        {
                return true;
	        }
            return false;          
        }

        public IXbimMeshGeometry3D GetVisibleMeshGeometry3D(int entityLabel, short modelId)
        {
            IEnumerable<XbimMeshFragment> fragments = GetMeshFragments(entityLabel); //get all the fragments for this entity in the visible layer
            int maxSize = fragments.Sum(f => f.PositionCount);
            XbimMeshGeometry3D geometry = new XbimMeshGeometry3D(maxSize);
            foreach (var fragment in fragments)
            {
                IXbimMeshGeometry3D geom = Visible.GetMeshGeometry3D(fragment);
                geometry.Add(geom, fragment.EntityLabel, fragment.EntityType, modelId);
            } 
            return geometry;
        }
        /// <summary>
        /// Returns all the layers including sub layers of this layer
        /// </summary>
        public IEnumerable<XbimMeshLayer<TVISIBLE, TMATERIAL>> Layers
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
                System.Diagnostics.Debug.WriteLine("Too big");
            }
            foreach (var layer in SubLayers)
            {
                layer.Balance();
            }
        }

        /// <summary>
        /// Useful for analysis and debugging purposes (invoked by Querying interface)
        /// </summary>
        /// <param name="Indentation">the number of indentation spaces at this tree level</param>
        /// <returns>Enumerable strings of indented elements</returns>
        public IEnumerable<string> LayersTree(int Indentation)
        {
            string s = new string(' ', Indentation);
            yield return s + this.name;
            foreach (var layer in SubLayers)
            {
                foreach (var item in layer.LayersTree(Indentation + 1))
                {
                    yield return item;
                } 
                // yield return layer.LayersTree(p + 1); 
            }
        }

        public void Add(String mesh, Type productType, int productLabel, int geometryLabel, XbimMatrix3D? transform = null, short modelId=0)
        {
            Hidden.Add(mesh, productType, productLabel, geometryLabel, transform, modelId);
        }
    }
}
