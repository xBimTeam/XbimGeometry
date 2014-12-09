#region XbimHeader

// The eXtensible Building Information Modelling (xBIM) Toolkit
// Solution:    XbimComplete
// Project:     Xbim.ModelGeometry.Scene
// Filename:    Rect3DExtensions.cs
// Published:   01, 2012
// Last Edited: 10:01 AM on 04 01 2012
// (See accompanying copyright.rtf)

#endregion

#region Directives

using System.IO;
using System.Windows.Media.Media3D;

#endregion

namespace Xbim.ModelGeometry.Scene
{
    public static class Rect3DExtensions
    {
        /// <summary>
        /// Reinitialises the rectangle 3d from the byte array
        /// </summary>
        /// <param name="rect"></param>
        /// <param name="array">6 doubles, definine, min and max values of the boudning box</param>
        public static Rect3D FromArray(this Rect3D rect, byte[] array)
        {
            MemoryStream ms = new MemoryStream(array);
            BinaryReader bw = new BinaryReader(ms);

            double srXmin = bw.ReadDouble();
            double srYmin = bw.ReadDouble();
            double srZmin = bw.ReadDouble();
            double srXmax = bw.ReadDouble();
            double srYmax = bw.ReadDouble();
            double srZmax = bw.ReadDouble();
            rect.Location = new Point3D(srXmin, srYmin, srZmin);
            rect.SizeX = srXmax - srXmin;
            rect.SizeY = srYmax - srYmin;
            rect.SizeZ = srZmax - srZmin;
            return rect;
        }

        static public Rect3D Inflate(this Rect3D rect, double x, double y, double z)
        {
            rect.X -= x; rect.Y -= y; rect.Z -= z;
            rect.SizeX += x * 2; rect.SizeY += y * 2; rect.SizeZ += z * 2;
            return rect;
        }

        static public Rect3D Inflate(this Rect3D rect, double d)
        {
            rect.X -= d; rect.Y -= d; rect.Z -= d;       
            rect.SizeX += d * 2; rect.SizeY += d * 2; rect.SizeZ += d * 2; 
            return rect;
        }

        static public Rect3D TransformBy(this Rect3D rect, Matrix3D matrix3d)
        {
            MatrixTransform3D m3d = new MatrixTransform3D(matrix3d);
            return m3d.TransformBounds(rect);
            
        }

        public static void Write(this Rect3D rect, BinaryWriter strm)
        {
            if (rect.IsEmpty)
                strm.Write('E');
            else
            {
                strm.Write('R');
                strm.Write(rect.X);
                strm.Write(rect.Y);
                strm.Write(rect.Z);
                strm.Write(rect.SizeX);
                strm.Write(rect.SizeY);
                strm.Write(rect.SizeZ);
            }
        }

        /// <summary>
        /// Calculates the centre of the 3D rect
        /// </summary>
        /// <param name="rect3D"></param>
        /// <returns></returns>
        public static Point3D Centroid(this Rect3D rect3D)
        {
            return new Point3D((rect3D.X + rect3D.SizeX / 2), (rect3D.Y + rect3D.SizeY / 2), (rect3D.Z + rect3D.SizeZ / 2));
        }



        public static Rect3D Read(this Rect3D rect, BinaryReader strm)
        {
            char test = strm.ReadChar();
            if (test == 'E')
                return new Rect3D();
            else
            {
                rect.X = strm.ReadDouble();
                rect.Y = strm.ReadDouble();
                rect.Z = strm.ReadDouble();
                rect.SizeX = strm.ReadDouble();
                rect.SizeY = strm.ReadDouble();
                rect.SizeZ = strm.ReadDouble();
                return rect;
            }
        }
    }
}