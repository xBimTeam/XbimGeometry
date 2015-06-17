using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
    /// <summary>
    /// Used in the clustering analysis of elements the model.
    /// </summary>
    public class XbimRegion
    {
        public string Name;
        public XbimVector3D Size;
        public XbimPoint3D Centre;
        public int Population = -1;

        public XbimRegion(string name, XbimRect3D bounds, int population)
        {
            this.Name = name;
            this.Size = new XbimVector3D(bounds.SizeX,bounds.SizeY,bounds.SizeZ);
            this.Centre = bounds.Centroid();
            this.Population = population;
        }

        public XbimRegion()
        {
        }

        public XbimRect3D ToXbimRect3D()
        {
            return new XbimRect3D(
                this.Centre - this.Size * 0.5,
                this.Size);
        }

        internal double Diagonal()
        {
            return Size.Length;
        }

        public XbimMatrix3D WorldCoordinateSystem { get; set; }
    }
}
