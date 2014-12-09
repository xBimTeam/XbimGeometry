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
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.IO;
using Xbim.Ifc2x3.Extensions;
using Xbim.Ifc2x3.GeometricConstraintResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.Kernel;
using Xbim.Ifc2x3.RepresentationResource;
using Xbim.XbimExtensions;
using Xbim.XbimExtensions.Interfaces;
using System.Collections.Concurrent;
using System.Threading.Tasks;
using Xbim.Ifc2x3.ProductExtension;

#endregion

namespace Xbim.ModelGeometry.Scene
{
    
    [Serializable]
    public class TransformGraph
    {
        private TransformNode _root;
        private Dictionary<long, TransformNode> _productNodes = new Dictionary<long, TransformNode>(); 
        private Dictionary<IfcObjectPlacement, TransformNode> _placementNodes =
            new Dictionary<IfcObjectPlacement, TransformNode>();


        [NonSerialized]
        private XbimModel _model;

        public XbimModel Model
        {
            get { return _model; }
          
        }

        public Dictionary<long, TransformNode> ProductNodes
        {
            get { return _productNodes; }
            internal set { _productNodes = value; }
        }



        public TransformGraph(XbimModel model)
        {
            _model = model;
            _root = new TransformNode();
        }
        
       

        public TransformNode Root
        {
            get { return _root; }
        }

        public TransformNode this[IfcProduct product]
        {
            get
            {
                IfcObjectPlacement pl = product.ObjectPlacement;
                if (pl != null)
                {
                    TransformNode node;
                    if (_placementNodes.TryGetValue(pl, out node))
                        return node;
                }
                return null;
            }
        }

        /// <summary>
        /// Add a set of products, to process the whole model use AddAllProducts for increased performance
        /// </summary>
        /// <typeparam name="TProduct"></typeparam>
        /// <param name="products"></param>
        public void AddProducts<TProduct>(IEnumerable<TProduct> products) where TProduct : IfcProduct
        {
            foreach (var product in products)
            {
                AddNode(product.ObjectPlacement, product);
            }
        }

        public void AddAllProducts()
        {

           List<XbimInstanceHandle> productHandles = ((XbimInstanceCollection)_model.Instances).Handles<IfcProduct>().ToList();
           foreach (var handle in productHandles)
            {
                IfcProduct product = _model.Instances[handle.EntityLabel] as IfcProduct;
                if(!(product is IfcFeatureElement)) //don't store openings and additions
                    AddNode(product.ObjectPlacement, product);
            }
            

        }


        public TransformNode AddProduct(IfcProduct product)
        {
            return AddNode(product.ObjectPlacement, product);
        }

        private TransformNode AddNode(IfcObjectPlacement placement, IfcProduct product)
        {
            IfcLocalPlacement lp = placement as IfcLocalPlacement;
            IfcGridPlacement gp = placement as IfcGridPlacement;
            if (gp != null) throw new NotImplementedException("GridPlacement is not implemented");
            if (lp != null)
            {
                
                TransformNode node;
                if (!_placementNodes.TryGetValue(placement, out node))
                {                 
                    node = new TransformNode(product);
                    if (product != null) _productNodes.Add(product.EntityLabel, node);
                    IfcAxis2Placement3D ax3 = lp.RelativePlacement as IfcAxis2Placement3D;
                    if (ax3 != null) node.LocalMatrix = ax3.ToMatrix3D();
                    else
                    {
                        IfcAxis2Placement2D ax2 = lp.RelativePlacement as IfcAxis2Placement2D;
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
                else // we might have created the node as a placement parent but not set the product yet
                {
                    if (product != null && node.ProductLabel == null) //don't add a product twice
                    {
                        node.ProductLabel = product.EntityLabel;
                        _productNodes.Add(product.EntityLabel, node);
                        return node;
                    }
                }
            }
            return null;
        }


      


    }
}