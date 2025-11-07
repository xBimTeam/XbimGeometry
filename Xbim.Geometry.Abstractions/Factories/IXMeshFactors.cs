namespace Xbim.Geometry.Abstractions
{
    public enum MeshGranularity
    {
        Course,
        Normal,
        Fine
    }

    public interface IXMeshFactors
    {
        /// <summary>
        /// The multipler to convert model units to meters, i.e. if the model is in millimeters, multiplier = 1000
        /// </summary>
        double OneMeter { get; set; }
        /// <summary>
        /// Measure in model units. A linear deflection of 4mm will give an accurate result for most models, 
        /// increase for courser meshes, this factor has a strong impact on the courseness of the mesh
        /// For building elements like curved walls that have a large bounding box, use larger numbers, but use a higher angular deflection 
        /// For products like toilets and sinks with a bounding box of around 1mx1mx1m use a lower angular deflection to retain a sensible visual representation
        /// </summary>
        double LinearDefection { get; set; }
        /// <summary>
        /// Measure in radians. Should be between 12 and 20 degrees, for BIMs generally this relates to LOD
        /// Setting it higher reducses the mesh size, small objects with a lot of detail can be rendered to high level of detail  by setting the
        /// linear deflection to 4 and the angular to 12, to reduce the level of detail increase the abgular deflection to up to 20, to make the mesh course 
        /// regardless of the netities bounding box increase the linear deflection.
        /// </summary>
        double AngularDeflection { get; set; }
        bool Relative { get; set; }
        //bool InternalVerticesMode { get; set; }
        //bool ControlSurfaceDeflection { get; set; }

        /// <summary>
        /// The distance between two points that determines if the points are equal
        /// </summary>
        double Tolerance { get; set; }

        /// <summary>
        /// changes the tolerances to the required granularity, returns the modified mesh factors
        /// </summary>
        /// <param name="granularity"></param>
        /// <returns></returns>
        IXMeshFactors SetGranularity(MeshGranularity granularity);
    }
}
