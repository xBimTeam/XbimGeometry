using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.ModelGeometry.Scene.Extensions
{
    public static class IIfcAxis2PlacementExtensions
    {
        public static XbimMatrix3D ToMatrix3D(this IIfcAxis2Placement placement)
        {
              
            var ax3 = placement as IIfcAxis2Placement3D;
            var ax2 = placement as IIfcAxis2Placement2D;
            if (ax3 != null)
                return ax3.ToMatrix3D();
            return ax2 != null ? ax2.ToMatrix3D() : XbimMatrix3D.Identity;
        }
    
        

        public static XbimMatrix3D ToMatrix3D(this IIfcAxis2Placement2D axis2)
        {
            if (axis2.RefDirection != null)
            {
                var v = new XbimVector3D(axis2.RefDirection.X, axis2.RefDirection.Y, axis2.RefDirection.Z);
                v = v.Normalized();
                return new XbimMatrix3D(v.X, v.Y, 0, 0, v.Y, v.X, 0, 0, 0, 0, 1, 0, axis2.Location.X, axis2.Location.Y, 0, 1);
            }
            return new XbimMatrix3D(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, axis2.Location.X, axis2.Location.Y,
                axis2.Location.Z, 1);
        }

        public static XbimMatrix3D ToMatrix3D(this IIfcAxis2Placement3D axis3)
        {
            if (axis3.RefDirection != null && axis3.Axis != null)
            {
                var za = new XbimVector3D(axis3.Axis.X,axis3.Axis.Y,axis3.Axis.Z);
                za = za.Normalized();
                var xa = new XbimVector3D(axis3.RefDirection.X, axis3.RefDirection.Y, axis3.RefDirection.Z); 
                xa = xa.Normalized();
                XbimVector3D ya = XbimVector3D.CrossProduct(za, xa);
                ya = ya.Normalized();
                return new XbimMatrix3D(xa.X, xa.Y, xa.Z, 0, ya.X, ya.Y, ya.Z, 0, za.X, za.Y, za.Z, 0, axis3.Location.X,
                                    axis3.Location.Y, axis3.Location.Z, 1);
            }
            else if (axis3.Location != null)
            {
                return new XbimMatrix3D(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, axis3.Location.X, axis3.Location.Y,
                    axis3.Location.Z, 1);
            }
            return new XbimMatrix3D(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        }
    }
}
