﻿#region Directives

using System.Collections.Generic;
using System.Linq;
using Xbim.Common.Geometry;
using Xbim.Ifc2x3.Extensions;
using Xbim.Ifc2x3.GeometricConstraintResource;
using Xbim.IO;

#endregion

namespace Xbim.ModelGeometry.Scene
{
    public class XbimPlacementTree
    {
        /// <summary>
        ///     Builds a placement tree of all ifcLocalPlacements
        /// </summary>
        /// <param name="model"></param>
        /// <param name="adjustWcs">
        ///     If there is a single root displacement, this is removed from the tree and added to the World
        ///     Coordinate System. Useful for models where the site has been located into a geographical context
        /// </param>
        public XbimPlacementTree(XbimModel model, bool adjustWcs = true)
        {
            var rootNodes = new List<XbimPlacementNode>();
            var localPlacements = model.InstancesLocal.OfType<IfcLocalPlacement>(true).ToList();
            Nodes = new Dictionary<int, XbimPlacementNode>();
            foreach (var placement in localPlacements)
                Nodes.Add(placement.EntityLabel, new XbimPlacementNode(placement));
            foreach (var localPlacement in localPlacements)
            {
                if (localPlacement.PlacementRelTo != null) //resolve parent
                {
                    var xbimPlacement = Nodes[localPlacement.EntityLabel];
                    var xbimPlacementParent = Nodes[localPlacement.PlacementRelTo.EntityLabel];
                    xbimPlacement.Parent = xbimPlacementParent;
                    xbimPlacementParent.Children.Add(xbimPlacement);
                }
                else
                    rootNodes.Add(Nodes[localPlacement.EntityLabel]);
            }
            if (adjustWcs && rootNodes.Count == 1)
            {
                var root = rootNodes[0];
                WorldCoordinateSystem = root.Matrix;
                //make the children parentless
                foreach (var node in Nodes.Values.Where(node => node.Parent == root)) node.Parent = null;
                root.Matrix = new XbimMatrix3D(); //set the matrix to identity
            }
            //muliply out the matrices
            foreach (var node in Nodes.Values) node.ToGlobalMatrix();
        }

        public XbimMatrix3D WorldCoordinateSystem { get; private set; }

        private Dictionary<int, XbimPlacementNode> Nodes { get; set; }

        public XbimMatrix3D this[int placementLabel]
        {
            get { return Nodes[placementLabel].Matrix; }
        }

        public class XbimPlacementNode
        {
            private List<XbimPlacementNode> _children;
            private bool _isAdjustedToGlobal;

            public XbimPlacementNode(IfcLocalPlacement placement)
            {
                PlacementLabel = placement.EntityLabel;
                Matrix = placement.RelativePlacement.ToMatrix3D();
                _isAdjustedToGlobal = false;
            }

            public int PlacementLabel { get; private set; }
            public XbimMatrix3D Matrix { get; protected internal set; }

            public List<XbimPlacementNode> Children
            {
                get { return _children ?? (_children = new List<XbimPlacementNode>()); }
            }

            public XbimPlacementNode Parent { get; set; }

            internal void ToGlobalMatrix()
            {
                if (!_isAdjustedToGlobal && Parent != null)
                {
                    Parent.ToGlobalMatrix();
                    Matrix = Matrix*Parent.Matrix;
                }
                _isAdjustedToGlobal = true;
            }
        }
    }
}