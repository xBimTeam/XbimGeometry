using System.Collections.Generic;
using Xbim.Geometry.Abstractions;

namespace Xbim.Geometry.Engine.Interop.BRepExtensions
{
    public class ShapeEqualityComparer : IEqualityComparer<IXShape>
    {
        public bool Equals( IXShape x,  IXShape y)
        {
            if(x is null && y is null) return true;
            if(x is null || y is null) return false;
            return x.IsEqual(y);
        }

        public int GetHashCode(IXShape obj)
        {
            return obj.ShapeHashCode();
        }
    }
}
