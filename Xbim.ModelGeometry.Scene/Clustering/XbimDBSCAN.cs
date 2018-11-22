using System;
using System.Collections.Generic;
using System.Linq;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene.Clustering
{
    /// <summary>
    /// Initial ideas for this class derive from the Density-based spatial clustering of applications with noise (DBSCAN).
    /// http://en.wikipedia.org/wiki/DBSCAN
    /// Except the Noise portion has not been implemented.
    /// </summary>
    public static class XbimDbscan
    {
        public static List<XbimBBoxClusterElement> GetClusters(IEnumerable<XbimBBoxClusterElement> itemsToCluster, double eps)
        {
            if (itemsToCluster == null) 
                return null;
            var clusters = itemsToCluster.ToList();
            // eps *= eps; // square eps

            var lastCount = 0;
            while (clusters.Count != lastCount)
            {
                lastCount = clusters.Count;
                for (var i = 0; i < clusters.Count; i++)
                {
                    var baseItem = clusters[i];
                    // efficiency fix: because clusters.removeat cost is O(count-index) then we start from the end of the list
                    for (var j = clusters.Count - 1; j > i; j--)
                    {
                        if (ValidDistance(baseItem.Bound, clusters[j].Bound, eps))
                        {
                            baseItem.Add(clusters[j]);
                            clusters.RemoveAt(j);
                        }                         
                    }
                }
            }
            return clusters;
        }

        /// <summary>
        /// Looks at the maximum distance (between all axis) between two boxes and compares it with a specified threshold.
        /// </summary>
        /// <param name="r1">Bounding box 1</param>
        /// <param name="r2">Bounding box 2</param>
        /// <param name="eps">the threshold distance</param>
        /// <returns>True if the maximum distance is under the threshold.</returns>
        private static bool ValidDistance(XbimRect3D r1, XbimRect3D r2, double eps)
        {
            //if (r2.SizeZ > 2000000)
            //    Console.WriteLine("v big");
            var dx = AxisDistance(r1.X, r1.SizeX, r2.X, r2.SizeX);
            var dy = AxisDistance(r1.Y, r1.SizeY, r2.Y, r2.SizeY);
            var dz = AxisDistance(r1.Z, r1.SizeZ, r2.Z, r2.SizeZ);
            var max = Math.Max(Math.Max(dx, dy), dz);
            return (max < eps);
        }

        /// <summary>
        /// Distance along a single ax.
        /// </summary>
        /// <param name="c1">Starting coordinate of interval 1 along the axis</param>
        /// <param name="s1">Size of interval 1 along the axis</param>
        /// <param name="c2">Starting coordinate of interval 2 along the axis</param>
        /// <param name="s2">Size of interval 2 along the axis</param>
        /// <returns>A positive distance if segments don't overlap (a negative if they do)</returns>
        private static double AxisDistance(double c1, double s1, double c2, double s2)
        {
            if (c1 < c2)
                return c2 - (c1 + s1);
            return c1 - (c2 + s2);
        }
    }
}
