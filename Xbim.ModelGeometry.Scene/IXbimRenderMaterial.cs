using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Ifc2x3.PresentationAppearanceResource;
using Xbim.Ifc2x3.PresentationResource;

namespace Xbim.ModelGeometry.Scene
{
    /// <summary>
    /// Interface for grpahic card specific render materials or shaders
    /// </summary>
    public interface IXbimRenderMaterial
    {
        /// <summary>
        /// Call to ensure the native material has been created
        /// </summary>
        /// <param name="texture"></param>
       void CreateMaterial(XbimTexture texture);
        /// <summary>
        /// True if the native material has been created
        /// </summary>
       bool IsCreated { get; }
       string Description { get; }
    }
}
