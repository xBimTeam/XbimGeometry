using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;
using Xbim.Ifc2x3.Kernel;

namespace Xbim.ModelGeometry.Scene
{
    public interface IXbimScene
    {
        void Close();
        TransformGraph Graph { get; }
        XbimLOD LOD { get; set; }
    }
}
