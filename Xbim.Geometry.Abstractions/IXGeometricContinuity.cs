namespace Xbim.Geometry.Abstractions
{
    public enum IXGeometricContinuity
    {
        /// <summary>
        /// only geometric continuity.
        /// </summary>
        GeomAbs_C0,
        /// <summary>
        /// for each point on the curve, the tangent vectors "on the right" and "on the left" are collinear with the same orientation.
        /// </summary>
        GeomAbs_G1,
        /// <summary>
        /// continuity of the first derivative. The "C1" curve is also "G1" but, in addition, the tangent vectors " on the right" and "on the left" are equal.
        /// continuity of the first derivatives; any isoparametric (in U or V) of a surface "C1" is also "C1".
        /// </summary>
        GeomAbs_C1,
        /// <summary>
        /// for each point on the curve, the normalized normal vectors "on the right" and "on the left" are equal.
        /// for BSpline curves only; "on the right" and "on the left" of a knot the computation of the "main curvature radii" and the "main directions" (when they exist) gives the same result.
        /// </summary>
        GeomAbs_G2,
        /// <summary>
        /// continuity of the second derivative.
        /// </summary>
        GeomAbs_C2,
        /// <summary>
        /// continuity of the third derivative.
        /// </summary>
        GeomAbs_C3,
        /// <summary>
        /// continuity of any N-th derivative, whatever is the value given for N (infinite order of continuity). We may also say that a surface is "Ci" in u, and "Cj" in v to indicate the continuity of its derivatives up to the order i in the u parametric direction, and j in the v parametric direction.
        /// </summary>
        GeomAbs_CN
    }
}

