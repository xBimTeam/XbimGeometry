﻿using System;
using System.Collections.Concurrent;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.ModelGeometry.Scene.Extensions
{
    // ReSharper disable once InconsistentNaming
    public static class IIfcCartesianTransformExtensions
    {

        public static XbimMatrix3D ToMatrix3D(this IIfcCartesianTransformationOperator ct, ConcurrentDictionary<int, Object> maps = null)
        {
            if (ct is IIfcCartesianTransformationOperator3DnonUniform)
               return ((IIfcCartesianTransformationOperator3DnonUniform) ct).ToMatrix3D(maps);
            else if (ct is IIfcCartesianTransformationOperator3D)
                return ((IIfcCartesianTransformationOperator3D) ct).ToMatrix3D(maps);
            else throw new ArgumentException("ToMatrix3D", "ct");
        }

        /// <summary>
        ///   Builds a windows XbimMatrix3D from a CartesianTransformationOperator3D
        /// </summary>
        /// <param name="ct3D"></param>
        /// <param name="maps">An optional mapping dictionary</param>
        /// <returns></returns>
        public static XbimMatrix3D ToMatrix3D(this IIfcCartesianTransformationOperator3D ct3D, ConcurrentDictionary<int, Object> maps = null)
        {
            if (maps == null)
                return ConvertCartesianTranformOperator3D(ct3D);
            else
            {

                object transform;
                if (maps.TryGetValue(ct3D.EntityLabel, out transform)) //already converted it just return cached
                    return (XbimMatrix3D)transform;
                var matrix = ConvertCartesianTranformOperator3D(ct3D);
                maps.TryAdd(ct3D.EntityLabel, matrix);
                return matrix;
            }
        }

        private static XbimMatrix3D ConvertCartesianTranformOperator3D(IIfcCartesianTransformationOperator3D ct3D)
        {
            var m3D = ConvertCartesianTransform3D(ct3D);
            m3D.Scale(ct3D.Scl);
            return m3D;
        }

        /// <summary>
        ///   Builds a windows XbimMatrix3D from a CartesianTransformationOperator3DnonUniform
        /// </summary>
        /// <param name="ct3D"></param>
        /// <param name="maps"></param>
        /// <returns></returns>
        public static XbimMatrix3D ToMatrix3D(this IIfcCartesianTransformationOperator3DnonUniform ct3D, ConcurrentDictionary<int, Object> maps = null)
        {
            if (maps == null)
                return ConvertCartesianTransformationOperator3DnonUniform(ct3D);
            else
            {
                object transform;
                if (maps.TryGetValue(ct3D.EntityLabel, out transform)) //already converted it just return cached
                    return (XbimMatrix3D)transform;
                var matrix = ConvertCartesianTransformationOperator3DnonUniform(ct3D);
                maps.TryAdd(ct3D.EntityLabel, matrix);
                return matrix;
            }
        }

        private static XbimMatrix3D ConvertCartesianTransformationOperator3DnonUniform(IIfcCartesianTransformationOperator3DnonUniform ct3D)
        {
            XbimVector3D u3; //Z Axis Direction
            XbimVector3D u2; //X Axis Direction
            XbimVector3D u1; //Y axis direction
            if (ct3D.Axis3 != null)
            {
                var dir = ct3D.Axis3;
                u3 = new XbimVector3D(dir.X, dir.Y, dir.Z);
                u3 = u3.Normalized();
            }
            else
                u3 = new XbimVector3D(0, 0, 1);
            if (ct3D.Axis1 != null)
            {
                var dir = ct3D.Axis1;
                u1 = new XbimVector3D(dir.X, dir.Y, dir.Z);
                u1 = u1.Normalized();
            }
            else
            {
                var defXDir = new XbimVector3D(1, 0, 0);
                u1 = u3 != defXDir ? defXDir : new XbimVector3D(0, 1, 0);
            }
            var xVec = XbimVector3D.Multiply(XbimVector3D.DotProduct(u1, u3), u3);
            var xAxis = XbimVector3D.Subtract(u1, xVec);
            xAxis = xAxis.Normalized();

            if (ct3D.Axis2 != null)
            {
                var dir = ct3D.Axis2;
                u2 = new XbimVector3D(dir.X, dir.Y, dir.Z);
                u2 = u2.Normalized();
            }
            else
                u2 = new XbimVector3D(0, 1, 0);

            var tmp = XbimVector3D.Multiply(XbimVector3D.DotProduct(u2, u3), u3);
            var yAxis = XbimVector3D.Subtract(u2, tmp);
            tmp = XbimVector3D.Multiply(XbimVector3D.DotProduct(u2, xAxis), xAxis);
            yAxis = XbimVector3D.Subtract(yAxis, tmp);
            yAxis = yAxis.Normalized();
            u2 = yAxis;
            u1 = xAxis;

            double locZ = ct3D.LocalOrigin.Z;
            if (double.IsNaN(locZ))
                locZ = 0;
            var lo = new XbimPoint3D(ct3D.LocalOrigin.X, ct3D.LocalOrigin.Y, locZ); //local origin

            var matrix = new XbimMatrix3D(u1.X, u1.Y, u1.Z, 0,
                                           u2.X, u2.Y, u2.Z, 0,
                                           u3.X, u3.Y, u3.Z, 0,
                                           lo.X, lo.Y, lo.Z, 1);
            matrix.Scale(new XbimVector3D(ct3D.Scl, ct3D.Scl2, ct3D.Scl3));

            return matrix;
        }

        private static XbimMatrix3D ConvertCartesianTransform3D(IIfcCartesianTransformationOperator3D ct3D)
        {
            XbimVector3D u3; //Z Axis Direction
            XbimVector3D u2; //X Axis Direction
            XbimVector3D u1; //Y axis direction
            if (ct3D.Axis3 != null)
            {
                var dir = ct3D.Axis3;
                u3 = new XbimVector3D(dir.X, dir.Y, dir.Z);
                u3 = u3.Normalized();
            }
            else
                u3 = new XbimVector3D(0, 0, 1);
            if (ct3D.Axis1 != null)
            {
                var dir = ct3D.Axis1;
                u1 = new XbimVector3D(dir.X, dir.Y, dir.Z);
                u1 = u1.Normalized();
            }
            else
            {
                var defXDir = new XbimVector3D(1, 0, 0);
                u1 = u3 != defXDir ? defXDir : new XbimVector3D(0, 1, 0);
            }
            var xVec = XbimVector3D.Multiply(XbimVector3D.DotProduct(u1, u3), u3);
            var xAxis = XbimVector3D.Subtract(u1, xVec);
            xAxis = xAxis.Normalized();

            if (ct3D.Axis2 != null)
            {
                var dir = ct3D.Axis2;
                u2 = new XbimVector3D(dir.X, dir.Y, dir.Z);
                u2 = u2.Normalized();
            }
            else
                u2 = new XbimVector3D(0, 1, 0);

            var tmp = XbimVector3D.Multiply(XbimVector3D.DotProduct(u2, u3), u3);
            var yAxis = XbimVector3D.Subtract(u2, tmp);
            tmp = XbimVector3D.Multiply(XbimVector3D.DotProduct(u2, xAxis), xAxis);
            yAxis = XbimVector3D.Subtract(yAxis, tmp);
            yAxis = yAxis.Normalized();
            u2 = yAxis;
            u1 = xAxis;

            double localOriginZ = ct3D.LocalOrigin?.Z ?? 0.0;
            if (double.IsNaN(localOriginZ))
                localOriginZ = 0;

            var lo = new XbimPoint3D(ct3D.LocalOrigin?.X ?? 0.0, ct3D.LocalOrigin?.Y ?? 0.0, localOriginZ); //local origin

            return new XbimMatrix3D(u1.X, u1.Y, u1.Z, 0,
                                           u2.X, u2.Y, u2.Z, 0,
                                           u3.X, u3.Y, u3.Z, 0,
                                           lo.X, lo.Y, lo.Z, 1);
           
        }
    }
}


    