namespace Xbim.Geometry.Abstractions
{
    /// <summary>
    /// The concrete Ifc solid types
    /// </summary>
    public enum XSolidModelType
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
        IfcSectionedSolidHorizontal, // new in IFC4x1 Release Candidate 3
		IfcSurfaceCurveSweptAreaSolid,
        
        // Ifc4x3
        IfcDirectrixDerivedReferenceSweptAreaSolid
    }
}
