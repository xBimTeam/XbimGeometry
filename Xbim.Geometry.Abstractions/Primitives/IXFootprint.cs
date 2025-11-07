using System;
using System.Collections.Generic;
using System.IO;

namespace Xbim.Geometry.Abstractions
{
    using Polygon = IEnumerable<IXPolyLoop2d>;
    public enum SFAGeometryType : UInt32
    {
        Empty = 0,
        Point = 1,
        LineString = 2,
        Polygon = 3,
        MultiPoint = 4,
        MultiLineString = 5,
        MultiPolygon = 6,
        GeometryCollection = 7
    }

    public interface IXFootprint
    {
        /// <summary>
        /// A list of Polygons
        /// Polygon is a geometry with a positive area (two-dimensional); 
        /// a sequence of points form a closed, non-self intersecting ring; 
        /// the first ring denotes the exterior ring, zero or more subsequent rings denote holes in this exterior ring
        /// </summary>
        IEnumerable<Polygon> Bounds { get; }
        double MinZ { get; }
        double MaxZ { get;  }
        /// <summary>
        /// True if the bounds are a fairly accurate fit to the shape, false if a bounding box approximation has been used due to a rpocessing issue
        /// </summary>
        bool IsClose { get; }
        
        SFAGeometryType SfaGeometryType { get; }
        /// <summary>
        /// returns the footprint boundaries in the SFA WellKnown Text Format
        /// </summary>
        /// <returns></returns>
        void Write(BinaryWriter binaryWriter);
        void Write(TextWriter textWriter);

    }
}
