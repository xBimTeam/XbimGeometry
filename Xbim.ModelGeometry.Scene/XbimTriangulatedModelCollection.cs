using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimTriangulatedModelCollection : List<XbimTriangulatedModel>
    {
      
        public static readonly XbimTriangulatedModelCollection Empty;
        

        static XbimTriangulatedModelCollection()
        {
            Empty = new XbimTriangulatedModelCollection();    
        }

        public XbimTriangulatedModelCollection():base(1)
        {

        }

        public XbimRect3D Bounds
        {
            get
            {
                XbimRect3D bb = XbimRect3D.Empty;
                foreach (var item in this)
                {
                    if (bb.IsEmpty) bb = item.Bounds;
                    else bb.Union(item.Bounds);
                }
                return bb;
            }
        }


       
    }
}
