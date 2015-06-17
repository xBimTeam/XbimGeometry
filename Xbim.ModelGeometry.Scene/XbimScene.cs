using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.IO;
using System.Linq;
using System.Text;
using Xbim.IO;
using Xbim.XbimExtensions.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    [Serializable]
    public class XbimScene<TVISIBLE, TMATERIAL>
        where TVISIBLE : IXbimMeshGeometry3D, new()
        where TMATERIAL : IXbimRenderMaterial, new()
    {
        
        XbimMeshLayerCollection<TVISIBLE, TMATERIAL> _layers = new XbimMeshLayerCollection<TVISIBLE, TMATERIAL>();

        public XbimMeshLayerCollection<TVISIBLE, TMATERIAL> SubLayers
        {
            get { return _layers; }
            set { _layers = value; }
        }
       

        readonly XbimColourMap _layerColourMap;
        private XbimModel _model;

        public XbimModel Model
        {
            get { return _model; }
            set { _model = value; }
        }

        /// <summary>
        /// The colour map for this scene
        /// </summary>
        public XbimColourMap LayerColourMap
        {
            get { return _layerColourMap; }
        }
        /// <summary>
        /// Constructs a scene using the default IfcProductType colour map
        /// </summary>
        public XbimScene(XbimModel model)
            :this(model,new XbimColourMap())
        {
          
        }

        /// <summary>
        /// Constructs a scene, using the specfified colourmap
        /// </summary>
        /// <param name="colourMap"></param>
        public XbimScene(XbimModel model, XbimColourMap colourMap)
        {
            this._layerColourMap = colourMap;
            this._model = model;
        }

        /// <summary>
        /// Returns all the layers including sub layers of this scene
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
        /// Returns all layers and sublayers that have got some graphic content that is visible
        /// </summary>
        public IEnumerable<XbimMeshLayer<TVISIBLE, TMATERIAL>> VisibleLayers
        {
            get
            {
                foreach (var layer in _layers)
                {
                    if (layer.Visible.Meshes.Any()) yield return layer;
                    foreach (var subLayer in layer.SubLayers)
                    {
                        if (subLayer.Visible.Meshes.Any()) yield return subLayer;
                    }
                }

            }
        }
        /// <summary>
        /// Add the layer to the scene
        /// </summary>
        /// <param name="layer"></param>
        public void Add(XbimMeshLayer<TVISIBLE, TMATERIAL> layer)
        {
            if (string.IsNullOrEmpty(layer.Name)) //ensure a layer has a unique name if the user has not defined one
                layer.Name = "Layer " + _layers.Count();
            if (!_layers.Contains(layer))
                _layers.Add(layer);
        }


        /// <summary>
        /// Makes all meshes in all layers in the scene Hidden
        /// </summary>
        public void HideAll()
        {
            foreach (var layer in _layers)
                layer.HideAll();
        }

        /// <summary>
        /// Makes all meshes in all layers in the scene Visible
        /// </summary>
        public void ShowAll()
        {
            foreach (var layer in _layers)
                layer.ShowAll();
        }

        /// <summary>
        /// Retrieves all the mesh fragments for the specified entity in this scene
        /// </summary>
        /// <param name="entityLabel"></param>
        /// <returns></returns>
        public XbimMeshFragmentCollection GetMeshFragments(int entityLabel)
        {
            var fragments = new XbimMeshFragmentCollection();
            foreach (var layer in Layers)
                fragments.AddRange(layer.GetMeshFragments(entityLabel));
            return fragments;
        }


        /// <summary>
        /// Gets the geometry of an entity building it up from layers.
        /// </summary>
        /// <param name="entity">The entity instance</param>
        public IXbimMeshGeometry3D GetMeshGeometry3D(IPersistIfcEntity entity, short modelId)
        {
            var geometry = new XbimMeshGeometry3D();
            IModel m = entity.ModelOf;
            foreach (var layer in Layers)
            {
                // an entity model could be spread across many layers (e.g. in case of different materials)
                if(layer.Model == m)
                    geometry.Add(layer.GetVisibleMeshGeometry3D(entity.EntityLabel, modelId));
            }
            return geometry;
        }

        /// <summary>
        /// Resizes layers so that none has more than USHORT number of indices
        /// </summary>
        public void Balance()
        {
            foreach (var layer in SubLayers)
            {
                layer.Balance();
            }
        }
    }
}
