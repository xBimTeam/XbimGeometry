#nullable enable
using System;
using System.Collections.Generic;
using Xbim.Common;
using Xbim.Ifc4.Interfaces;
using Xbim.ModelGeometry.Scene.Extensions;

namespace Xbim.ModelGeometry.Scene
{
	[Obsolete("Use XbimPlacementTree instead")]
	[Serializable]
	public class TransformGraph
	{
		private readonly TransformNode _root;
		private Dictionary<long, TransformNode> _productNodes = new();
		private readonly Dictionary<IIfcObjectPlacement, TransformNode> _placementNodes = new();

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

		public TransformNode? this[IIfcProduct product]
		{
			get
			{
				var pl = product.ObjectPlacement;
				if (pl == null)
					return null;
				return _placementNodes.TryGetValue(pl, out TransformNode? node)
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

		public void AddAllProducts(bool includeFeatureProducts = false)
		{
			var productHandles = _model.Instances.OfType<IIfcProduct>();
			foreach (var product in productHandles)
			{
				//don't store openings and additions
				if (!includeFeatureProducts && product is IIfcFeatureElement)
					continue;
				AddNode(product.ObjectPlacement, product);
			}
		}

		public TransformNode AddProduct(IIfcProduct product)
		{
			if (product is null)
				throw (new ArgumentNullException(nameof(product)));
			if (_productNodes.ContainsKey(product.EntityLabel))
				return _productNodes[product.EntityLabel];
			return AddNode(product.ObjectPlacement, product);
		}

		private TransformNode AddNode(IIfcObjectPlacement placement, IIfcProduct? product)
		{
			if (_placementNodes.TryGetValue(placement, out TransformNode? node))
			{
				// we reuse the existing node, but we swap the product label
				if (product != null)
				{
					node.ProductLabel = product.EntityLabel;
					if (!_productNodes.ContainsKey(product.EntityLabel))
						_productNodes.Add(product.EntityLabel, node);
				}
				return node;
			}

			if (placement is IIfcLocalPlacement lp)
			{
				node = new TransformNode(product);
				if (lp.RelativePlacement is IIfcAxis2Placement3D ax3)
					node.LocalMatrix = ax3.ToMatrix3D();
				else if (lp.RelativePlacement is IIfcAxis2Placement2D ax2)
					node.LocalMatrix = ax2.ToMatrix3D();
				_placementNodes.Add(placement, node);

				var par = GetParent(lp);
				par.AddChild(node);
				node.Parent = par;
			}

			// todo: other forms of placement are more complicated and might require geometry engine

			if (node is not null)
			{
				if (product is not null && !_productNodes.ContainsKey(product.EntityLabel))
					_productNodes.Add(product.EntityLabel, node);
				return node;
			}
			throw new NotImplementedException($"{placement.GetType().Name} is not implemented.");
		}

		private TransformNode GetParent(IIfcLocalPlacement lp)
		{
			if (lp.PlacementRelTo == null) //it is in world coordinate system, add it to root
				return _root;
			if (_placementNodes.TryGetValue(lp.PlacementRelTo, out TransformNode? parent)) //we have already processed the parent
				return parent;
			return AddNode(lp.PlacementRelTo, null); //make sure placement tree is created
		}
	}
}
#nullable restore