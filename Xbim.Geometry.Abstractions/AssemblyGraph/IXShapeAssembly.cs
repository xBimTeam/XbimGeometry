using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
   
    public interface IXShapeAssembly : IXShapeComponent
    {
        IEnumerable<IXShapeInstance> Instances { get; }
        void AddInstance(IXShapeInstance component);
        IEnumerable<IXShapeAssembly> Openings { get; }
        IEnumerable<IXShapeAssembly> Projections { get; }
        bool IsSubAssembly { get; set; }
        
    }
}
