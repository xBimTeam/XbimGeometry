using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Ifc;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimRenderMaterial: IXbimRenderMaterial
    {
        public void CreateMaterial(XbimTexture texture)
        {
            throw new NotImplementedException();
        }

        public bool IsCreated
        {
            get { throw new NotImplementedException(); }
        }

        public string Description
        {
            get
            {
                return "Not Implemented";
            }
        }
    }
}
