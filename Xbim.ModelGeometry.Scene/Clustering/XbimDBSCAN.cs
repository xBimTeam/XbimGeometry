using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene.Clustering
{
    /// <summary>
    /// Initial ideas for this class derive from the Density-based spatial clustering of applications with noise (DBSCAN).
    /// http://en.wikipedia.org/wiki/DBSCAN
    /// Except the Noise portion has bot been implemented.
    /// </summary>
    public static class XbimDBSCAN
    {
        public static List<XbimBBoxClusterElement> GetClusters(IEnumerable<XbimBBoxClusterElement> ItemsToCluster, double eps)
        {
            if (ItemsToCluster == null) 
                return null;
            List<XbimBBoxClusterElement> clusters = ItemsToCluster.ToList();
            // eps *= eps; // square eps

            int LastCount = 0;
            while (clusters.Count != LastCount)
            {
                LastCount = clusters.Count;
                for (int i = 0; i < clusters.Count; i++)
                {
                    XbimBBoxClusterElement BaseItem = clusters[i];
                    for (int j = i+1; j < clusters.Count; j++)
                    {
                        if (ValidDistance(BaseItem.Bound, clusters[j].Bound, eps))
                        {
                            BaseItem.Add(clusters[j]);
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
        /// <param name="R1">Bounding box 1</param>
        /// <param name="R2">Bounding box 2</param>
        /// <param name="eps">the threshold distance</param>
        /// <returns>True if the maximum distance is under the threshold.</returns>
        private static bool ValidDistance(Common.Geometry.XbimRect3D R1, Common.Geometry.XbimRect3D R2, double eps)
        {
            double dx = AxisDistance(R1.X, R1.SizeX, R2.X, R2.SizeX);
            double dy = AxisDistance(R1.Y, R1.SizeY, R2.Y, R2.SizeY);
            double dz = AxisDistance(R1.Z, R1.SizeZ, R2.Z, R2.SizeZ);
            double max = Math.Max(Math.Max(dx, dy), dz);
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
            else
                return c1 - (c2 + s2);
        }

    }
}
