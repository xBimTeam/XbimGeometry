using System.Collections.Generic;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeService
    {

        /// <summary>
        /// Unifies faces and edges of the shape which lie on the same geometry
        /// </summary>
        /// <param name="shape"></param>
        /// <returns>modified copy of the shape</returns>
        IXShape UnifyDomain(IXShape shape);
        IXShape Convert(string brepString);
        IXbimGeometryObject ConvertToV5(string brepString);
        string Convert(IXShape shape);
        string Convert(IXbimGeometryObject v5Shape);


        IXShape Transform(IXShape shape, IXMatrix transformMatrix);
        void Triangulate(IXShape shape);
        IXShape Union(IXShape body, IXShape addition, double precision);
        IXShape Cut(IXShape body, IXShape subtraction, double precision);
        IXShape Intersect(IXShape body, IXShape intersect, double precision);
        IXShape Union(IXShape body, IEnumerable<IXShape> additions, double precision);
        IXShape Cut(IXShape body, IEnumerable<IXShape> subtractions, double precision);
        IXShape Intersect(IXShape body, IEnumerable<IXShape> intersections, double precision);
        /// <summary>
        /// Removes any placement
        /// </summary>
        /// <param name="shape"></param>
        /// <returns></returns>
        IXShape RemovePlacement(IXShape shape);
        /// <summary>
        /// Sets the placement of the shape, overrides any existing placement
        /// </summary>
        /// <param name="shape"></param>
        /// <param name="placement"></param>
        /// <returns></returns>
        IXShape SetPlacement(IXShape shape, IIfcObjectPlacement placement);


        /// <summary>
        /// Applies the inverse of the object placement if invertPacement is true
        /// </summary>
        /// <param name="shape"></param>
        /// <param name="placement"></param>
        /// <param name="invertPlacement">if true inverts the placement tranformation that is applied</param>
        /// <returns></returns>
        IXShape Moved(IXShape shape, IIfcObjectPlacement placement, bool invertPlacement = false);
        IXShape Moved(IXShape shape, IXLocation moveTo);
        IXShape Scaled(IXShape shape, double scale);

        bool IsFacingAwayFrom(IXFace face, IXDirection direction);


        /// <summary>
        /// Creates a single compound shape from the shapes params
        /// </summary>
        /// <param name="shapes"></param>
        /// <returns></returns>
        IXShape Combine(IEnumerable<IXShape> shapes);

        /// <summary>
        /// Determines whether the first shape is overlapping with the second shape.
        /// </summary>
        /// <param name="shape1">The first shape.</param>
        /// <param name="shape2">The second shape.</param>
        /// <param name="meshFactors">The mesh factors used in the internal shapes proximity test.</param>
        /// <returns>
        ///   <c>true</c> if the specified shape1 is overlapping with shape2; otherwise, <c>false</c>.
        /// </returns>
        bool IsOverlapping(IXShape shape1, IXShape shape2, IXMeshFactors meshFactors);

        byte[] CreateWexBimMesh(IXShape shape, double tolerance, double linearDeflection, double angularDeflection, double scale, out IXAxisAlignedBoundingBox bounds);

    }
}
