namespace Xbim.Geometry.Abstractions
{
    /// <summary>
    /// The concrete Ifc solid types
    /// </summary>
    public enum  XSolidModelType
    {
        IfcCsgSolid,
        IfcSweptDiskSolid,
        IfcSweptDiskSolidPolygonal,
        IfcAdvancedBrep,
        IfcAdvancedBrepWithVoids,
        IfcFacetedBrep,
        IfcFacetedBrepWithVoids,
        IfcExtrudedAreaSolid,
        IfcExtrudedAreaSolidTapered,
        IfcFixedReferenceSweptAreaSolid, 
        IfcRevolvedAreaSolid,
        IfcRevolvedAreaSolidTapered,
        IfcSurfaceCurveSweptAreaSolid
    }
}
