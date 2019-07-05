#region XbimHeader

// The eXtensible Building Information Modelling (xBIM) Toolkit
// Solution:    XbimComplete
// Project:     Xbim.ModelGeometry.Scene
// Filename:    TransformGraph.cs
// Published:   01, 2012
// Last Edited: 9:05 AM on 20 12 2011
// (See accompanying copyright.rtf)

#endregion

#region Directives
using System;
using System.Collections.Generic;
using Xbim.Common;
using Xbim.Ifc4.Interfaces;
using Xbim.ModelGeometry.Scene.Extensions;

#endregion

namespace Xbim.ModelGeometry.Scene
{
    
    [Serializable]
    public class TransformGraph
    {
        private TransformNode _root;
        private Dictionary<long, TransformNode> _productNodes = new Dictionary<long, TransformNode>(); 
        private readonly Dictionary<IIfcObjectPlacement, TransformNode> _placementNodes =
            new Dictionary<IIfcObjectPlacement, TransformNode>();


        [NonSerialized]
        private readonly IModel _model;

        public IModel Model
        {
            get { return _model; }
        }

        public Dictionary<long, TransformNode> ProductNodes
        {
            get { return _productNodes; }
            internal set { _productNodes = value; }
        }

        public TransformGraph(IModel model)
        {
            _model = model;
            _root = new TransformNode();
        }

        public TransformNode Root
        {
            get { return _root; }
        }

        public TransformNode this[IIfcProduct product]
        {
            get
            {
                var pl = product.ObjectPlacement;
                if (pl == null) 
                    return null;
                TransformNode node;
                return _placementNodes.TryGetValue(pl, out node) 
                    ? node 
                    : null;
            }
        }

        /// <summary>
        /// Add a set of products, to process the whole model use AddAllProducts for increased performance
        /// </summary>
        /// <typeparam name="TProduct"></typeparam>
        /// <param name="products"></param>
        public void AddProducts<TProduct>(IEnumerable<TProduct> products) where TProduct : IIfcProduct
        {
            foreach (var product in products)
            {
                AddNode(product.ObjectPlacement, product);
            }
        }

        public void AddAllProducts()
        {
            var productHandles = _model.Instances.OfType<IIfcProduct>();
            foreach (var product in productHandles)
            {              
                if (!(product is IIfcFeatureElement)) //don't store openings and additions
                    AddNode(product.ObjectPlacement, product);
            }
        }


        public TransformNode AddProduct(IIfcProduct product)
        {
            return AddNode(product.ObjectPlacement, product);
        }

        private TransformNode AddNode(IIfcObjectPlacement placement, IIfcProduct product)
        {
            var lp = placement as IIfcLocalPlacement;
            var gp = placement as IIfcGridPlacement;
            if (gp != null) 
                throw new NotImplementedException("GridPlacement is not implemented");
            if (lp == null) 
                return null;
            TransformNode node;
            if (!_placementNodes.TryGetValue(placement, out node))
            {                 
                node = new TransformNode(product);
                if (product != null) _productNodes.Add(product.EntityLabel, node);
                var ax3 = lp.RelativePlacement as IIfcAxis2Placement3D;
                if (ax3 != null) 
                    node.LocalMatrix = ax3.ToMatrix3D();
                else
                {
                    var ax2 = lp.RelativePlacement as IIfcAxis2Placement2D;
                    if (ax2 != null) node.LocalMatrix = ax2.ToMatrix3D();
                }

                _placementNodes.Add(placement, node);
                if (lp.PlacementRelTo != null)
                {
                    TransformNode parent;
                    if (_placementNodes.TryGetValue(lp.PlacementRelTo, out parent))
                        //we have already processed parent
                    {
                        parent.AddChild(node);
                        node.Parent = parent;
                    }
                    else //make sure placement tree is created
                    {
                        parent = AddNode(lp.PlacementRelTo, null);
                        parent.AddChild(node);
                        node.Parent = parent;
                    }
                }
                else //it is in world coordinate system just add it
                {
                    _root.AddChild(node);
                    node.Parent = _root;
                }
                return node;
            }

            if (product == null || node.ProductLabel != null) 
                return null;
            node.ProductLabel = product.EntityLabel;
            _productNodes.Add(product.EntityLabel, node);
            return node;
        }
    }
}