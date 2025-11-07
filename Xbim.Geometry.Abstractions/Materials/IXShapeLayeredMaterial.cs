using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeLayeredMaterial : IXShapeMaterialItem
    {
        IList<IXShapeMaterialLayer> MaterialLayerList { get; }
        
    }
}
