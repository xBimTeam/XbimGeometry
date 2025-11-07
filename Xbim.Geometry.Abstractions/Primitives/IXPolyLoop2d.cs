using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXPolyLoop2d : IEnumerable<IXPoint>
    {
        /// <summary>
        /// Builds a wire in the Z plane
        /// </summary>
        /// <param name="zDim">Z plane value, default 0.</param>
        /// <returns></returns>
        IXWire BuildWire(double zDim = 0);
        int NbPoints { get; }
    }
}
