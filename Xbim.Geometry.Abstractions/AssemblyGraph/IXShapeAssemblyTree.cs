using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeAssemblyTree
    {
        int Id { get; set; }
        IEnumerable<IXShapeAssemblyTree> Children { get; }
        bool HasChildren { get; }
        IXShapeAssemblyTree AddChild(int id);
        void AddChild(IXShapeAssemblyTree child);
    }
}
