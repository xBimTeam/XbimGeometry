using System;

namespace Xbim.Tessellator.MeshSimplification
{
    /// <summary>
    /// Garland‑Heckbert formulation of quadric error metric in the (A, b, c) form.
    /// </summary>
    public class Quadric
    {
        private const double EPSILON = 1e-10;
        private double _Axx, _Axy, _Axz, _Ayy, _Ayz, _Azz;     // matrix A
        private double _bx, _by, _bz; // Linear term –b
        private double _c; // Constant term

        public Quadric()
        {
        }

        public static readonly Quadric Zero = new Quadric
        {
            _Axx = 0,
            _Axy = 0,
            _Axz = 0,
            _Ayy = 0,
            _Ayz = 0,
            _Azz = 0,
            _bx = 0,
            _by = 0,
            _bz = 0,
            _c = 0
        };

        /// <summary>
        /// Construct a quadric from a plane defined by the normal and a point on the plane.
        /// </summary>
        public static Quadric FromPlane(in Vec3 normal, in Vec3 planePoint)
        {
            var Q = new Quadric
            {
                _Axx = normal.X * normal.X,
                _Axy = normal.X * normal.Y,
                _Axz = normal.X * normal.Z,
                _Ayy = normal.Y * normal.Y,
                _Ayz = normal.Y * normal.Z,
                _Azz = normal.Z * normal.Z
            };

            Q._bx = Q._by = Q._bz = Q._c = 0;

            // b = -A p,
            // c = pᵀ A p
            double x = Q._Axx * planePoint.X + Q._Axy * planePoint.Y + Q._Axz * planePoint.Z;
            double y = Q._Axy * planePoint.X + Q._Ayy * planePoint.Y + Q._Ayz * planePoint.Z;
            double z = Q._Axz * planePoint.X + Q._Ayz * planePoint.Y + Q._Azz * planePoint.Z;
            var v = new Vec3(x, y, z);
            Q._bx = -v.X;
            Q._by = -v.Y;
            Q._bz = -v.Z;
            var pnt = planePoint;
            Vec3.Dot(ref pnt, ref v, out Q._c);
            return Q;
        }

        public Quadric Scale(double scale) => new Quadric
        {
            _Axx = _Axx * scale,
            _Axy = _Axy * scale,
            _Axz = _Axz * scale,
            _Ayy = _Ayy * scale,
            _Ayz = _Ayz * scale,
            _Azz = _Azz * scale,
            _bx = _bx * scale,
            _by = _by * scale,
            _bz = _bz * scale,
            _c = _c * scale
        };

        /// <summary>
        /// Evaluate vᵀQv for an arbitrary point v.
        /// </summary>
        public double Evaluate(in Vec3 v)
        {
            double x = _Axx * v.X + _Axy * v.Y + _Axz * v.Z;
            double y = _Axy * v.X + _Ayy * v.Y + _Ayz * v.Z;
            double z = _Axz * v.X + _Ayz * v.Y + _Azz * v.Z;

            return v.X * x + v.Y * y + v.Z * z +
                   2.0 * (v.X * _bx + v.Y * _by + v.Z * _bz) + _c;
        }

        /// <summary>
        /// Multiply the matrix A by vector p
        /// </summary>
        public Vec3 MultiplyMatrix(in Vec3 p)
        {
            return new Vec3(
                _Axx * p.X + _Axy * p.Y + _Axz * p.Z,
                _Axy * p.X + _Ayy * p.Y + _Ayz * p.Z,
                _Axz * p.X + _Ayz * p.Y + _Azz * p.Z
            );
        }

        /// <summary>
        /// Finds the optimal point for edge contraction between two vertices.
        /// If the quadric matrix is singular, evaluates the error at both endpoints and the midpoint,
        /// returning the point with the lowest error.
        /// </summary>
        public Vec3 Optimal(in Vec3 v0, in Vec3 v1)
        {
            if (Minimize(out Vec3 result))
                return result;

            // If singular matrix, evaluate error at endpoints and midpoint
            Vec3 midPoint = new Vec3()
            {
                X = (v0.X + v1.X) * 0.5,
                Y = (v0.Y + v1.Y) * 0.5,
                Z = (v0.Z + v1.Z) * 0.5,
            };

            double errorV0 = Evaluate(v0);
            double errorV1 = Evaluate(v1);
            double errorMid = Evaluate(midPoint);

            // Return the point with minimum error
            if (errorV0 <= errorV1 && errorV0 <= errorMid)
                return v0;
            else if (errorV1 <= errorV0 && errorV1 <= errorMid)
                return v1;
            else
                return midPoint;
        }

        /// <summary>
        /// Compute the point that minimises the quadric error.
        /// Returns <c>false</c> when the matrix A is singular.
        /// </summary>
        public bool Minimize(out Vec3 result)
        {
            double a11 = _Ayy * _Azz - _Ayz * _Ayz;
            double a12 = _Axz * _Ayz - _Azz * _Axy;
            double a13 = _Axy * _Ayz - _Axz * _Ayy;
            double a22 = _Azz * _Axx - _Axz * _Axz;
            double a23 = _Axy * _Axz - _Axx * _Ayz;
            double a33 = _Axx * _Ayy - _Axy * _Axy;

            double det = _Axx * a11 + _Axy * a12 + _Axz * a13;

            if (Math.Abs(det) > 1000.0 * EPSILON)
            {
                double invDet = 1.0 / det;

                a11 *= invDet; a12 *= invDet; a13 *= invDet;
                a22 *= invDet; a23 *= invDet; a33 *= invDet;

                double x = a11 * _bx + a12 * _by + a13 * _bz;
                double y = a12 * _bx + a22 * _by + a23 * _bz;
                double z = a13 * _bx + a23 * _by + a33 * _bz;

                result = new Vec3(-x, -y, -z);
                return true;
            }

            result = default;
            return false;
        }

        public static Quadric operator +(in Quadric q1, in Quadric q2)
        {
            return new Quadric()
            {
                _Axx = q1._Axx + q2._Axx,
                _Axy = q1._Axy + q2._Axy,
                _Axz = q1._Axz + q2._Axz,
                _Ayy = q1._Ayy + q2._Ayy,
                _Ayz = q1._Ayz + q2._Ayz,
                _Azz = q1._Azz + q2._Azz,

                _bx = q1._bx + q2._bx,
                _by = q1._by + q2._by,
                _bz = q1._bz + q2._bz,

                _c = q1._c + q2._c
            };
        }
    }
}