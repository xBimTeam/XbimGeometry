using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXSolid : IXShape
    {
        public string BRepDefinition();
        IEnumerable<IXShell> Shells { get; }
    }
}
