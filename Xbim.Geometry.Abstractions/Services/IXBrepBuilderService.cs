namespace Xbim.Geometry.Abstractions
{
    /// <summary>
    /// Service for building Brep IShapes using IX interfaces only
    /// </summary>
    public interface IXBrepBuilderService : IXModelScoped
    {


        /// <summary>
        /// Adds one or more edges to a wire and returns a new wire;
        /// </summary>
        /// <param name="wire">base wire, this paramters is unchanged</param>
        /// <param name="edges"></param>
        /// <returns>nb. it is possible to build non-compliant shapes, no checks are made</returns>
        IXWire Add(IXWire wire, params IXEdge[] edges);

        /// <summary>
        /// Adds one or more wires to a face and returns a new face containing these as outer and inner bounds
        /// </summary>
        /// <param name="face">base face, this paramters is unchanged</param>
        /// <param name="wires"></param>
        /// <returns>nb. it is possible to build non-compliant shapes, no checks are made</returns>

        IXFace Add(IXFace face, params IXWire[] wires);
        /// <summary>
        /// Adds a set of faces to a shell and returns a new shell
        /// </summary>
        /// <param name="shell">base shell, this paramters is unchanged</param>
        /// <param name="faces"></param>
        /// <returns>nb. it is possible to build non-compliant shapes, no checks are made</returns>
        IXShell Add(IXShell shell, params IXFace[] faces);
        /// <summary>
        /// Adds a set of shells to a solid and returns a new solid
        /// </summary>
        /// <param name="solid">base solid, this paramters is unchanged</param>
        /// <param name="shells"></param>
        /// <returns>nb. it is possible to build non-compliant shapes, no checks are made</returns>    
        IXSolid Add(IXSolid solid, params IXShell[] shells);

        /// <summary>
        /// Adds a set of shapes to a compound and returns a new compound
        /// </summary>
        /// <param name="compound"></param>
        /// <param name="shapes"></param>
        /// <returns>nb. it is possible to build non-compliant shapes, no checks are made</returns>
        IXCompound Add(IXCompound compound, params IXShape[] shapes);


        IXPoint BuildVertex(double x, double y, double z = 0);


        /// <summary>
        /// Build a linear edge between the two points
        /// </summary>
        /// <param name="start"></param>
        /// <param name="end"></param>
        /// <returns></returns>
        IXEdge BuildEdge(IXPoint start, IXPoint end);

        /// <summary>
        /// Builds a new wire from a set of edges
        /// </summary>
        /// <param name="edges"></param>
        /// <returns></returns>        
        IXWire BuildWire(params IXEdge[] edges);

        /// <summary>
        /// Builds a new wire comprising linear segments (polyloop) from a set of points
        /// To close the wire repeat the start point at the end
        /// </summary>
        /// <param name="points"></param>
        /// <returns></returns>        
        IXWire BuildWire(params IXPoint[] points);

        /// <summary>
        /// Builds a new face from a set of wires
        /// </summary>
        /// <param name="surface">All wires must lie on this surface within model tolerance</param>
        /// <param name="wires"></param>
        /// <returns></returns>
        IXFace BuildFace(IXSurface surface, params IXWire[] wires);

        /// <summary>
        /// Builds a new shell from a set of faces
        /// </summary>
        /// <param name="faces"></param>
        /// <returns></returns>       
        IXShell BuildShell(params IXFace[] faces);

        /// <summary>
        /// Builds a new solid from a set of shells
        /// </summary>
        /// <param name="shells"></param>
        /// <returns></returns>
        IXSolid BuildSolid(params IXShell[] shells);

        /// <summary>
        /// Builds a new compound containing all of the shapes
        /// </summary>
        /// <param name="shapes">shapes to add to the new compound</param>
        /// <returns></returns>
        IXCompound BuildCompound(params IXShape[] shapes);

        #region Geometry Objects
        /// <summary>
        /// Builds a 3D point
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="z"></param>
        /// <returns></returns>
        IXPoint BuildPoint(double x, double y, double z = 0);
        IXDirection BuildDirection(double x, double y, double z = 0);

        IXPlane BuildPlane(IXPoint origin, IXDirection normal);


        #endregion
    }
}
