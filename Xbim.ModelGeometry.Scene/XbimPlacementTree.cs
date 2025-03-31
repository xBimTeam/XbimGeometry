using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.Interfaces;
using Xbim.ModelGeometry.Scene.Extensions;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimPlacementTree
    {
        /// <summary>
        /// This function centralises the extraction of a product placement, but it needs the support of XbimPlacementTree and an XbimGeometryEngine
        /// We should probably find a conceptual place for it somewhere in the scene, where these are cached.
        /// </summary>
        public static XbimMatrix3D GetTransform(IIfcProduct product, XbimPlacementTree tree, IXbimGeometryEngine engine)
        {
            XbimMatrix3D placementTransform = XbimMatrix3D.Identity;
            if (product.ObjectPlacement is IIfcLocalPlacement)
                placementTransform = tree[product.ObjectPlacement.EntityLabel];
            else if (product.ObjectPlacement is IIfcGridPlacement gridPlacament)
                placementTransform = engine.ToMatrix3D(gridPlacament, null);
            else if (product.ObjectPlacement is Ifc4x3.GeometricConstraintResource.IfcLinearPlacement linearPlacement)
                placementTransform = engine.ToMatrix3D(linearPlacement, null);
            return placementTransform;
        }

        /// <summary>
        ///     Builds a placement tree of all ifcLocalPlacements
        /// </summary>
        /// <param name="model"></param>
        /// <param name="engine"></param>
        /// <param name="adjustWcs">
        ///     If there is a single root displacement, this is removed from the tree and added to the World
        ///     Coordinate System. Useful for models where the site has been located into a geographical context
        /// </param>
        public XbimPlacementTree(IModel model, IXbimGeometryEngine engine, bool adjustWcs = true)
        {
            var rootNodes = new List<XbimPlacementNode>();
            var localPlacements = model.Instances.OfType<IIfcLocalPlacement>(true).ToList();
            var linearPlacements = model.Instances.OfType<Xbim.Ifc4x3.GeometricConstraintResource.IfcLinearPlacement>(true).ToList();

            Nodes = new Dictionary<int, XbimPlacementNode>();
            foreach (var placement in localPlacements)
                Nodes.Add(placement.EntityLabel, new XbimPlacementNode(placement));

            foreach (var placement in linearPlacements)
            {
                var placementTransform = engine.ToMatrix3D(placement, null);
                Nodes.Add(placement.EntityLabel, new XbimPlacementNode(placement.EntityLabel, placementTransform));
            }

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

        [DebuggerDisplay("#{PlacementLabel} = {Matrix}, {HasParent} / {ChildrenCount}")]
        public class XbimPlacementNode
        {
            private List<XbimPlacementNode> _children;
            private bool _isAdjustedToGlobal;

            public XbimPlacementNode(IIfcLocalPlacement placement)
            {
                PlacementLabel = placement.EntityLabel;
                Matrix = placement.RelativePlacement.ToMatrix3D();
                _isAdjustedToGlobal = false;
            }

            public XbimPlacementNode(int placementId, XbimMatrix3D placementMatrix)
            {
                PlacementLabel = placementId;
                Matrix = placementMatrix;
                _isAdjustedToGlobal = false;
            }

            public int PlacementLabel { get; private set; }
            public XbimMatrix3D Matrix { get; protected internal set; }

            public List<XbimPlacementNode> Children
            {
                get { return _children ?? (_children = new List<XbimPlacementNode>()); }
            }

            public int ChildrenCount => Children.Count;

            public bool HasParent => Parent != null;

            public XbimPlacementNode Parent { get; set; }

            internal void ToGlobalMatrix()
            {
                if (!_isAdjustedToGlobal && Parent != null)
                {
                    Parent.ToGlobalMatrix();
                    Matrix = Matrix * Parent.Matrix;
                }
                _isAdjustedToGlobal = true;
            }
        }
    }
}