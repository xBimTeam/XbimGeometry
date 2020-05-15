using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXShell: IXShape
    {
        IEnumerable<IXFace> Faces { get; }
    }
}
