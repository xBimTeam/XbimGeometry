using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public  interface IXCompoundFactory : IXModelScoped
    {
        IXCompound CreateEmpty();
        IXCompound CreateFrom(IEnumerable<IXShape> shapes);
    }
}
