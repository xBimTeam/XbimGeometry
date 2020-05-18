using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXSolid : IXShape
    {
      
        IEnumerable<IXShell> Shells { get; }       
    }
}
