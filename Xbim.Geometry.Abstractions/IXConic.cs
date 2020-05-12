using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    public interface IXConic: IXCurve
    {
        /// <summary>
        /// Positional axis depends on dimensionality, if the circle is a 2d circle it will be IXAxis2Placement2d, a 3d circle will be IXAxis2Placement3d
        /// </summary>
        IXAxisPlacement Position { get; }
    }
}
