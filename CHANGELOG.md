# Changelog

All notable changes to this project should be documented in this file

## [v5.1.239] 2019-06-03

Candidate release for 5.1

### Changed
- AssemblyVersion changed to 5.1.0.0
- Faceted FaceSet re-implemented for better tolerances
- Threading model changed to avoid race conditions. Boolean Cut operations optimised
- Composite curve reimplemented to correctly handle polylines as edges
- Half space solid reimplemented with OCC MakeHalfSpaceSolid

### Added
- Support for IfcIndexedPolyCurve [#180](https://github.com/xBimTeam/XbimGeometry/issues/180) 
- Support for IfcPolygonalFaceSet [#106](https://github.com/xBimTeam/XbimGeometry/issues/106) 
- Support for 2D Polyline Curves

### Removed

### Fixed
- Access Violation cutting xBimSolid [#177](https://github.com/xBimTeam/XbimGeometry/issues/177) 
- Very large polygonal meshes causing handled to prevent Out of Memory and performance issues [#176](https://github.com/xBimTeam/XbimGeometry/issues/176) 
- Fix for some circular opening cutting issues [#178](https://github.com/xBimTeam/XbimGeometry/issues/178)
- Fix for hanging with near infinite solid extrusions [#160](https://github.com/xBimTeam/XbimGeometry/issues/160)
- Fix exceptions calculating grids [#95](https://github.com/xBimTeam/XbimGeometry/issues/95) [#135](https://github.com/xBimTeam/XbimGeometry/issues/135)
- Fix for zero grid bounding box
- Tolerance fixes in cutting - opening not being cut [#166](https://github.com/xBimTeam/XbimGeometry/issues/166)
- BooleanClippingResult not cut correctly - (5.0 regression) [#158](https://github.com/xBimTeam/XbimGeometry/issues/158)
- Fix for SurfaceBasedModels defined being as multiple models. Fix for imprecise planar wires [#73](https://github.com/xBimTeam/XbimGeometry/issues/73)
- Render IfcTriangulatedFaceSet correctly [#145](https://github.com/xBimTeam/XbimGeometry/issues/145) / [#167](https://github.com/xBimTeam/XbimGeometry/issues/167)
- Extruded area solids with compound profiles return a solid
- Fix for inacurately defined Polyloops that are not planar
- Fix for orientation of trimmed composite curve segments that are reversed
- IfcSweptDiskSolidPolygonal fixed for closed directrix
- XbimWire trim fixed for incorrect parameterisation
- IfcSectionedSpine fixed incorrect orientation error
- Handle errors caused by Solids with two coincidental faces
- Fix for trimming of compound curve wire
- Support faulty solids with zero volume
- Closed sweep for SweptSolid fixed
- Fix pipe maker tolerance issue
- Polygonal bounded half space corrected for potential boolean hangs with large extremes
- Edge start and end points fixed to handle null vertices
- Empty profile definition handled
- Polygonally bounded half-space extrusion limited to prevent boolean hangs
- Composite curve creation handles incorrect reverse segment definition

## [v5.0.163] 2018-12-17 Nuget Release

### Changed

### Added

### Removed

### Fixed
