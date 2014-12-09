#region XbimHeader

// The eXtensible Building Information Modelling (xBIM) Toolkit
// Solution:    XbimComplete
// Project:     Xbim.ModelGeometry.Scene
// Filename:    CoPlanarComparer.cs
// Published:   01, 2012
// Last Edited: 9:05 AM on 20 12 2011
// (See accompanying copyright.rtf)

#endregion

#region Directives

using System.Collections.Generic;
using System.Windows.Media.Media3D;

#endregion

namespace Xbim.ModelGeometry.Scene
{
    public class CoPlanarComparer : IEqualityComparer<Vector3D>
    {
        public double AngleTolerance = 1;

        /// <summary>
        ///   Constructs a coplanar comparer with a default tolerance of 1 degree
        /// </summary>
        public CoPlanarComparer()
        {
        }

        public CoPlanarComparer(double angleTolerance)
        {
            AngleTolerance = angleTolerance;
        }

        #region IEqualityComparer<Vector3D> Members

        public bool Equals(Vector3D x, Vector3D y)
        {
            return Vector3D.AngleBetween(x, y) < AngleTolerance;
        }

        public int GetHashCode(Vector3D obj)
        {
            return obj.GetHashCode();
        }

        #endregion
    }
}