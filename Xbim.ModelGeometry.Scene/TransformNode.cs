#region XbimHeader

// The eXtensible Building Information Modelling (xBIM) Toolkit
// Solution:    XbimComplete
// Project:     Xbim.ModelGeometry.Scene
// Filename:    TransformNode.cs
// Published:   01, 2012
// Last Edited: 9:05 AM on 20 12 2011
// (See accompanying copyright.rtf)

#endregion

#region Directives

using System;
using System.Collections.Generic;
using System.Linq;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;


#endregion

namespace Xbim.ModelGeometry.Scene
{
    [Serializable]
    public class TransformNode
    {
        private XbimMatrix3D _localMatrix;
        private HashSet<TransformNode> _children;
        private int? _productLabel;
        private TransformNode _parent;

        public IEnumerable<TransformNode> Children
        {
            get { return _children ?? Enumerable.Empty<TransformNode>(); }
        }

        public int ChildCount
        {
            get { return _children == null ? 0 : _children.Count; }
        }

        public TransformNode()
        {
        }

        public TransformNode(IIfcProduct prod)        
        {
            if(prod!=null)
                _productLabel = prod.EntityLabel;
        }

       

        public int? ProductLabel
        {
            get
            {
                return _productLabel;
            }
            set { _productLabel = value; }
        }

        public IIfcProduct NearestProduct(IModel model)
        {
            if (!_productLabel.HasValue && _parent != null)
                return _parent.NearestProduct(model);
            else if (_productLabel.HasValue)
                return model.Instances[_productLabel.Value] as IIfcProduct;
            else
                return null;
        }

        public IIfcProduct Product(IModel model)
        {
            if (_productLabel.HasValue)
                return model.Instances[_productLabel.Value] as IIfcProduct;
            else
                return null;
        }
        public TransformNode Parent
        {
            get { return _parent; }
            set { _parent = value; }
        }

        public void AddChild(TransformNode child)
        {
            if (_children == null) _children = new HashSet<TransformNode>();
            _children.Add(child);
          
        }

        public void RemoveChild(TransformNode child)
        {
            if (_children != null)
            {
                _children.Remove(child);
            }
        }

        public XbimMatrix3D WorldMatrix()
        {
            if (_parent != null)
                return _localMatrix*_parent.WorldMatrix();
            else
                return _localMatrix;
        }


        public XbimMatrix3D LocalMatrix
        {
            get { return _localMatrix; }
            set { _localMatrix = value; }
        }

       

    }
}