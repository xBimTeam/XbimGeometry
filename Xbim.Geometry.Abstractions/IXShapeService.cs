using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeService
    {
        /// <summary>
        /// Unifies faces and edges of the shape which lie on the same geometry
        /// </summary>
        /// <param name="shape"></param>
        /// <returns>modified copy of the shape</returns>
        IXShape UnifyDomain(IXShape shape);
    }
}
