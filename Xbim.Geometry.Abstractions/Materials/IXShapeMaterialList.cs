using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeMaterialList : IXShapeMaterialItem
    {
        public List<IXShapeMaterialItem> Materials { get; set; }
    }
}
