#include "SurfaceFactory.h"
#include "GeometryFactory.h"
#include "CurveFactory.h"
#include "WireFactory.h"
#include "ProfileFactory.h"
#include "BIMAuthoringToolWorkArounds.h"
#include "EdgeFactory.h"
#include "../BRep/XFace.h"
#include "../BRep/XPlane.h"
#include "../BRep/XCylindricalSurface.h"
#include "../BRep/XConicalSurface.h"
#include "../BRep/XSurfaceOfRevolution.h"
#include "../BRep/XSurfaceOfLinearExtrusion.h"
#include "../BRep/XSectionedSurface.h"

#include <TopoDS_Edge.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <TopoDS.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <ShapeFix_Edge.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <GeomAPI_PointsToBSplineSurface.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepFill.hxx>
#include <BRepBuilderAPI_Sewing.hxx>

using namespace Xbim::Geometry::BRep;
using namespace Xbim::Ifc4x3::ProfileResource;
using namespace System;


namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
#pragma region Interfaces 

			IXPlane^ SurfaceFactory::BuildPlane(IXPoint^ origin, IXDirection^ normal)
			{
				Handle(Geom_Plane) occPlane = Ptr()->BuildPlane(origin->X, origin->Y, origin->Z, normal->X, normal->Y, normal->Z);
				if (occPlane.IsNull())
					throw RaiseGeometryFactoryException("Error building plane");
				else
					return gcnew XPlane(occPlane);
			}

			IXSurface^ SurfaceFactory::Build(IIfcSurface^ ifcSurface)
			{
				XSurfaceType surfaceType;
				if (!Enum::TryParse<XSurfaceType>(ifcSurface->ExpressType->ExpressName, surfaceType))
					throw RaiseGeometryFactoryException("Unsupported surface type", ifcSurface);


				if (surfaceType == XSurfaceType::IfcCurveBoundedPlane)
				{
					IIfcCurveBoundedPlane^ curveBoundedPlane = static_cast<IIfcCurveBoundedPlane^>(ifcSurface);
					TopoDS_Face face = BuildCurveBoundedPlane(curveBoundedPlane);
					if (!face.IsNull()) 
						return gcnew XFace(face); 
					else throw RaiseGeometryFactoryException("Failed to build surface", ifcSurface);
				}
				else if (surfaceType == XSurfaceType::IfcCurveBoundedSurface)
				{
					IIfcCurveBoundedSurface^ curveBoundedSurface = static_cast<IIfcCurveBoundedSurface^>(ifcSurface);
					TopoDS_Face face = BuildCurveBoundedSurface(curveBoundedSurface);
					if (!face.IsNull()) 
						return gcnew XFace(face); 
					else throw RaiseGeometryFactoryException("Failed to build surface", ifcSurface);
				}
				else if (surfaceType == XSurfaceType::IfcSectionedSurface)
				{
					IfcSectionedSurface^ curveBoundedSurface = static_cast<IfcSectionedSurface^>(ifcSurface);
					Handle(Geom_Curve) directrix;
					XCurveType directrixType;
					TopoDS_Shape surface = BuildSectionedSurface(curveBoundedSurface, directrix, directrixType);
					if (!surface.IsNull()) {
						XSectionedSurface^ sectionedSurface = gcnew XSectionedSurface(surface);
						sectionedSurface->Directrix = CURVE_FACTORY->BuildXCurve(directrix, directrixType);
						return sectionedSurface;
					}
					else throw RaiseGeometryFactoryException("Failed to build surface", ifcSurface);
				}
				else
				{
					Handle(Geom_Surface) surface = BuildSurface(ifcSurface, surfaceType);
					if (surface.IsNull())
						throw RaiseGeometryFactoryException("Error building surface", ifcSurface);
					return XSurface::GeomToXSurface(surface);
				}
			}

#pragma endregion

#pragma region OCC

			Handle(Geom_Surface) SurfaceFactory::BuildSurface(IIfcSurface^ ifcSurface, XSurfaceType surfaceType)
			{

				switch (surfaceType)
				{
					case XSurfaceType::IfcBSplineSurfaceWithKnots:
						return BuildBSplineSurfaceWithKnots(static_cast<IIfcBSplineSurfaceWithKnots^>(ifcSurface));
					case XSurfaceType::IfcRationalBSplineSurfaceWithKnots:
						return BuildRationalBSplineSurfaceWithKnots(static_cast<IIfcRationalBSplineSurfaceWithKnots^>(ifcSurface));
					case XSurfaceType::IfcCurveBoundedPlane:
					case XSurfaceType::IfcCurveBoundedSurface:
						throw RaiseGeometryFactoryException("Curve Bounded Surfaces must be built as faces", ifcSurface);
					case XSurfaceType::IfcRectangularTrimmedSurface:
						return BuildRectangularTrimmedSurface(static_cast<IIfcRectangularTrimmedSurface^>(ifcSurface));
					case XSurfaceType::IfcSurfaceOfLinearExtrusion:
						return BuildSurfaceOfLinearExtrusion(static_cast<IIfcSurfaceOfLinearExtrusion^>(ifcSurface));
					case XSurfaceType::IfcSurfaceOfRevolution:
						return BuildSurfaceOfRevolution(static_cast<IIfcSurfaceOfRevolution^>(ifcSurface));
					case XSurfaceType::IfcCylindricalSurface:
						return BuildCylindricalSurface(static_cast<IIfcCylindricalSurface^>(ifcSurface));
					case XSurfaceType::IfcPlane:
						return BuildPlane(static_cast<IIfcPlane^>(ifcSurface));
					case XSurfaceType::IfcSectionedSurface:
						throw RaiseGeometryFactoryException("Sectioned Surfaces must be built as shapes", ifcSurface);
					case XSurfaceType::IfcSphericalSurface:
						throw RaiseGeometryFactoryException("Surface of type is not implemented", ifcSurface);
					case XSurfaceType::IfcToroidalSurface:
						throw RaiseGeometryFactoryException("Surface of type is not implemented", ifcSurface);
					default:
						throw RaiseGeometryFactoryException("Surface of type is not implemented", ifcSurface);
				}

			}

			TopoDS_Shape SurfaceFactory::BuildSectionedSurface(IfcSectionedSurface^ sectionedSurface, Handle(Geom_Curve)% directrix, XCurveType% directrixType)
			{ 
				if (sectionedSurface->CrossSectionPositions->Count != sectionedSurface->CrossSections->Count)
			 		throw RaiseGeometryFactoryException("Number of cross-section position doesn't match the number of cross-sections", sectionedSurface);
			
				// Build Directrix
				directrix = CURVE_FACTORY->BuildCurve(sectionedSurface->Directrix, directrixType);
			
				// Build Sections
				std::vector<TopLoc_Location> locations;
				std::vector<std::vector<TaggedPoint>> allPoints;

				for (int i = 0; i < sectionedSurface->CrossSections->Count; i++)
				{
					IfcOpenCrossProfileDef^ section = static_cast<IfcOpenCrossProfileDef^>(sectionedSurface->CrossSections[i]);
					if (section == nullptr)
						throw RaiseGeometryFactoryException("IfcSectionedSurface section should be of type IfcOpenCrossProfileDef");

					TopLoc_Location location;
					GEOMETRY_FACTORY->ToLocation(sectionedSurface->CrossSectionPositions[i], location);
					locations.push_back(location);

					std::vector<double> widths;
					std::vector<double> slopes;
					std::vector<std::string> tags;
					bool horizontal = section->HorizontalWidths;

					for (int i = 0; i < section->Widths->Count; i++)
					{
						// Widths and Slopes should be the same count
						Xbim::Ifc4x3::MeasureResource::IfcNonNegativeLengthMeasure^ width = section->Widths[i];
						Xbim::Ifc4x3::MeasureResource::IfcPlaneAngleMeasure^ slope = section->Slopes[i];
						widths.push_back(static_cast<double>(width->Value));
						slopes.push_back(static_cast<double>(slope->Value));
					} 

					if (section->Tags->Count > 0) {
						for (int i = 0; i < section->Tags->Count; i++)
						{
							Xbim::Ifc4x3::MeasureResource::IfcLabel^ tag = section->Tags[i];
							const char* tagLabel = static_cast<const char*>
								(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(tag->Value->ToString()).ToPointer());
							std::string stdString(tagLabel);
							tags.push_back(stdString);
						}
					}
					else {
						for (int i = 1; i <= static_cast<int>(widths.size()) + 1; ++i) {
							tags.push_back(std::to_string(i));
						}
					}

					const std::vector<TaggedPoint>& points = BuildPolylinePoints(widths, slopes, tags, horizontal);
					allPoints.push_back(points);
				}
			
				bool uniform = IsUniform(allPoints);

				if (!uniform)
				{
					size_t maxSize = 0;
					for (const auto& points : allPoints) {
						maxSize = std::max(maxSize, points.size());
					}

					// resize vectors by duplicating the point before the missing tag
					for (size_t i = 0; i < allPoints.size() - 1; ++i) {
						std::vector<TaggedPoint>& currentVec = allPoints[i];
						const std::vector<TaggedPoint>& nextVec = allPoints[i + 1];

						while (currentVec.size() < maxSize && nextVec.size() != currentVec.size()) {
							for (size_t j = 1; j < nextVec.size(); ++j) {
								if (!ContainsTag(currentVec, nextVec[j].tag)) {
									currentVec.insert(currentVec.begin() + j, TaggedPoint(currentVec[j - 1].point, nextVec[j].tag));
									break;
								}
							}
						}

						if (i > 0) {
							std::vector<TaggedPoint>& prevVec = allPoints[i - 1];
							while (currentVec.size() < maxSize && prevVec.size() != currentVec.size()) {
								for (size_t j = 1; j < prevVec.size(); ++j) {
									if (!ContainsTag(currentVec, prevVec[j].tag)) {
										currentVec.insert(currentVec.begin() + j, TaggedPoint(currentVec[j - 1].point, prevVec[j].tag));
										break;
									}
								}
							}
						}
					}
				}

				std::vector<std::string> tags;
				std::vector<TopoDS_Shape> surfaces;

				for (const auto& taggedPoint : allPoints[0]) {
					tags.push_back(taggedPoint.tag);
				}

				for (size_t i = 0; i < tags.size() - 1; ++i)
				{
					BRepBuilderAPI_Sewing sewingTool;

					std::string currenTag = tags[i];
					std::string nextTag = tags[i + 1];

					TopoDS_Wire wire1 = CreateWireFromTag(allPoints, locations, currenTag);
					TopoDS_Wire wire2 = CreateWireFromTag(allPoints, locations, nextTag);

					std::vector<TopoDS_Edge> edges1;
					std::vector<TopoDS_Edge> edges2;

					for (TopExp_Explorer explorer(wire1, TopAbs_EDGE); explorer.More(); explorer.Next()) {
						TopoDS_Edge edge = TopoDS::Edge(explorer.Current());
						edges1.push_back(edge);
					}

					for (TopExp_Explorer explorer(wire2, TopAbs_EDGE); explorer.More(); explorer.Next()) {
						TopoDS_Edge edge = TopoDS::Edge(explorer.Current());
						edges2.push_back(edge);
					}

					for (size_t i = 0; i < edges1.size(); ++i) {
						TopoDS_Shape ruledSurface = BRepFill::Face(edges1[i], edges2[i]);
						sewingTool.Add(ruledSurface);
					}

					sewingTool.Perform();
					TopoDS_Shape surface = sewingTool.SewedShape();
					surfaces.push_back(surface);
				}

				BRepBuilderAPI_Sewing sewingTool;
				for (const auto& shape : surfaces)
				{
					sewingTool.Add(shape);
				}

				sewingTool.Perform();

				std::ostringstream oss;
				oss << "DBRep_DrawableShape" << std::endl;
				BRepTools::Write(sewingTool.SewedShape(), oss);
				std::ofstream outFile("C:/Users/ibrah/OneDrive/Desktop/Surface.brep");
				outFile << oss.str();
				outFile.close();

				return sewingTool.SewedShape();

			}

			TopoDS_Wire SurfaceFactory::CreateWireFromTag
				(const std::vector<std::vector<TaggedPoint>>& allPoints, const std::vector<TopLoc_Location>& locations, const std::string& tag)
			{
				BRepBuilderAPI_MakePolygon polygon;
				size_t i = 0;

				for (const auto& vec : allPoints) {
					const TopLoc_Location& location = locations[i];
					for (const auto& taggedPoint : vec) {
						if (taggedPoint.tag == tag) {
							const gp_Pnt& transformed = taggedPoint.point.Transformed(location.Transformation());
							polygon.Add(transformed);
						}
					}
					i++;
				}

				return polygon.Wire();
			}

			bool SurfaceFactory::ContainsTag(const std::vector<TaggedPoint>& points, const std::string& tag)
			{
				for (size_t i = 0; i < points.size(); ++i) {
					if (points[i].tag == tag) {
						return true;
					}
				}
				return false;
			}

			TopoDS_Wire SurfaceFactory::CreateWireFromSection(const std::vector<TaggedPoint>& section, const TopLoc_Location& location)
			{
				BRepBuilderAPI_MakePolygon polygon;

				for (const auto& taggedPoint : section)
				{
					const gp_Pnt& transformed = taggedPoint.point.Transformed(location.Transformation());

					polygon.Add(transformed);
				}

				return polygon.Wire();
			}
 

			bool SurfaceFactory::IsUniform(std::vector<std::vector<TaggedPoint>>& allPoints)
			{
				if (allPoints.empty()) return true;
				size_t firstSize = allPoints.front().size();
				for (size_t i = 1; i < allPoints.size(); ++i)
				{
					if (allPoints[i].size() != firstSize)
						return false;
				}
				return true;
			}


			std::vector<TaggedPoint> SurfaceFactory::BuildPolylinePoints
				(const std::vector<double>& widths, const std::vector<double>& slopes, const std::vector<std::string>& tags, bool horizontalWidths)
			{
				gp_Pnt currentPoint(0, 0, 0);
				bool hasTags = tags.size() > 0 && tags.size() == widths.size() + 1;

				TaggedPoint currentTaggedPnt(currentPoint, hasTags? tags[0] : "");
				std::vector<TaggedPoint> points;
				points.push_back(currentTaggedPnt);

				for (size_t i = 0; i < widths.size(); ++i)
				{
					double width = widths[i];
					double slope = slopes[i];

					double dx, dy;

					if (horizontalWidths)
					{
						dx = width;
						dy = width * tan(slope);
					}
					else
					{
						dx = width * cos(slope);
						dy = width * sin(slope);
					}

					gp_Pnt nextPoint(currentPoint.X() + dx, currentPoint.Y() + dy, 0);
					TaggedPoint taggedPnt(nextPoint, hasTags? tags[i+1] : "");
					points.push_back(taggedPnt);
					currentPoint = nextPoint;
				}

				return points;
			}


			Handle(Geom_Plane) SurfaceFactory::BuildPlane(IIfcPlane^ ifcPlane)
			{
				return BuildPlane(ifcPlane, false);
			}

			Handle(Geom_Plane) SurfaceFactory::BuildPlane(IIfcPlane^ ifcPlane, bool snap)
			{
				gp_Pnt origin = GEOMETRY_FACTORY->BuildPoint3d(ifcPlane->Position->Location);
				gp_Vec normal = gp::DZ(); //default to (0,0,1)
				if (ifcPlane->Position->Axis != nullptr)
					if (!GEOMETRY_FACTORY->BuildDirection3d(ifcPlane->Position->Axis, normal))
						throw RaiseGeometryFactoryException("Plane axis is incorrectly defined", ifcPlane->Position->Axis);
				gp_Vec refDir = gp::DX();
				if (ifcPlane->Position->RefDirection!=nullptr && !GEOMETRY_FACTORY->BuildDirection3d(ifcPlane->Position->RefDirection, refDir))
					throw RaiseGeometryFactoryException("Plane reference direction is incorrectly defined", ifcPlane->Position->RefDirection);
				if (snap)
				{
					gp_Dir n = normal;
					if (n.IsEqual(gp::DZ(), 1e-4))
						normal = gp::DZ();
					else if (n.IsEqual(gp::DX(), 1e-4))
						normal = gp::DX();
					else if (n.IsEqual(gp::DY(), 1e-4))
						normal = gp::DY();
					else if (n.IsEqual(-gp::DZ(), 1e-4))
						normal = -gp::DZ();
					else if (n.IsEqual(-gp::DX(), 1e-4))
						normal = -gp::DX();
					else if (n.IsEqual(-gp::DY(), 1e-4))
						normal = -gp::DY();
				}
				Handle(Geom_Plane) plane = EXEC_NATIVE->BuildPlane(origin, normal, refDir);
				if (plane.IsNull())
					throw RaiseGeometryFactoryException("Plane is badly defined. See logs", ifcPlane);
				return plane;
			}

			Handle(Geom_SurfaceOfRevolution) SurfaceFactory::BuildSurfaceOfRevolution(IIfcSurfaceOfRevolution^ ifcSurfaceOfRevolution)
			{
				if (ifcSurfaceOfRevolution->SweptCurve->ProfileType != Xbim::Ifc4::Interfaces::IfcProfileTypeEnum::CURVE)
					throw RaiseGeometryFactoryException("Only profiles of type curve are valid in a surface of revolution.", ifcSurfaceOfRevolution->SweptCurve);
				XProfileDefType profileDefType;
				Handle(Geom_Curve) sweptEdge = PROFILE_FACTORY->BuildCurve(ifcSurfaceOfRevolution->SweptCurve, profileDefType); //throws exception
				if (sweptEdge.IsNull())
					throw RaiseGeometryFactoryException("IfcSurfaceOfRevolution swept edge is incorrectly defined", ifcSurfaceOfRevolution->SweptCurve);

				gp_Pnt origin = GEOMETRY_FACTORY->BuildPoint3d(ifcSurfaceOfRevolution->AxisPosition->Location);
				gp_Vec axisDir(0, 0, 1);
				if (ifcSurfaceOfRevolution->AxisPosition->Axis != nullptr)
				{
					if (!GEOMETRY_FACTORY->BuildDirection3d(ifcSurfaceOfRevolution->AxisPosition->Axis, axisDir))
						throw RaiseGeometryFactoryException("IfcSurfaceOfRevolution axis is incorrectly defined", ifcSurfaceOfRevolution->AxisPosition->Axis);
				}
				gp_Ax1 axis(origin, axisDir);
				Handle(Geom_SurfaceOfRevolution) revolutedSurface = new Geom_SurfaceOfRevolution(sweptEdge, axis);

				if (revolutedSurface.IsNull())
					throw RaiseGeometryFactoryException("Invalid IfcSurfaceOfRevolution", ifcSurfaceOfRevolution);
				else
					return revolutedSurface;
			}

			Handle(Geom_SurfaceOfLinearExtrusion) SurfaceFactory::BuildSurfaceOfLinearExtrusion(IIfcSurfaceOfLinearExtrusion^ ifcSurfaceOfLinearExtrusion)
			{
				if (ifcSurfaceOfLinearExtrusion->SweptCurve->ProfileType != Xbim::Ifc4::Interfaces::IfcProfileTypeEnum::CURVE)
					throw RaiseGeometryFactoryException("Only profiles of type curve are valid in a surface of linearExtrusion", ifcSurfaceOfLinearExtrusion);
				TopLoc_Location location;
				if (ifcSurfaceOfLinearExtrusion->Position != nullptr)
				{
					location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(ifcSurfaceOfLinearExtrusion->Position);
				}

				TopoDS_Edge sweptEdge;
				if (!BIM_WORKAROUNDS->FixRevitIncorrectArcCentreSweptCurve(ifcSurfaceOfLinearExtrusion, sweptEdge))
				{
					//didn't need a fix so just create it
					sweptEdge = PROFILE_FACTORY->BuildProfileEdge(ifcSurfaceOfLinearExtrusion->SweptCurve);
					if (sweptEdge.IsNull()) //the edge was an empty segment
					{
						LogDebug(ifcSurfaceOfLinearExtrusion->SweptCurve, "Swept Edge is invalid or empty");
						return Handle(Geom_SurfaceOfLinearExtrusion)();
					}
				}
				gp_Vec extrude;
				if (!GEOMETRY_FACTORY->BuildDirection3d(ifcSurfaceOfLinearExtrusion->ExtrudedDirection, extrude))
					throw RaiseGeometryFactoryException("Direction of IfcSurfaceOfLinearExtrusion is invalid", ifcSurfaceOfLinearExtrusion->ExtrudedDirection);
				extrude *= ifcSurfaceOfLinearExtrusion->Depth;
				BIM_WORKAROUNDS->FixRevitSweptSurfaceExtrusionInFeet(extrude);
				bool hasRevitBSplineIssue = BIM_WORKAROUNDS->FixRevitIncorrectBsplineSweptCurve(ifcSurfaceOfLinearExtrusion, sweptEdge);

				Handle(Geom_SurfaceOfLinearExtrusion) surface = EXEC_NATIVE->BuildSurfaceOfLinearExtrusion(sweptEdge, extrude, hasRevitBSplineIssue);
				if (surface.IsNull())
					throw RaiseGeometryFactoryException("Surface of IfcSurfaceOfLinearExtrusion is invalid", ifcSurfaceOfLinearExtrusion);

				if (!hasRevitBSplineIssue && !location.IsIdentity()) surface->Transform(location);
				return surface;
			}

			TopoDS_Face SurfaceFactory::BuildCurveBoundedPlane(IIfcCurveBoundedPlane^ ifcCurveBoundedPlane)
			{
				Handle(Geom_Plane) basisPlane = BuildPlane(ifcCurveBoundedPlane->BasisSurface); //throws an exception with any failure
				TopoDS_Wire outerBoundary = WIRE_FACTORY->BuildWire(ifcCurveBoundedPlane->OuterBoundary, true);//throws an exception with any failure
				BRepBuilderAPI_MakeFace  faceMaker(basisPlane, outerBoundary);

				for each (IIfcCurve ^ innerCurve in ifcCurveBoundedPlane->InnerBoundaries)
				{
					TopoDS_Wire innerBound = WIRE_FACTORY->BuildWire(innerCurve, false);//throws an exception with any failure
					faceMaker.Add(innerBound);
				}

				if (faceMaker.IsDone())
				{
					gp_Trsf trsf;
					trsf.SetTransformation(basisPlane->Position(), gp::XOY());
					TopoDS_Face face = faceMaker.Face();
					face.Move(trsf);
					ShapeFix_Edge sfe;
					for (TopExp_Explorer exp(faceMaker.Face(), TopAbs_EDGE); exp.More(); exp.Next())
					{
						sfe.FixAddPCurve(TopoDS::Edge(exp.Current()), faceMaker.Face(), Standard_False);
					}
					return face;
				}
				else
					throw RaiseGeometryFactoryException("Invalid curve bounded plane", ifcCurveBoundedPlane);

			}

			TopoDS_Face SurfaceFactory::BuildCurveBoundedSurface(IIfcCurveBoundedSurface^ ifcCurveBoundedSurface)
			{
				throw gcnew NotImplementedException();
			}

			Handle(Geom_RectangularTrimmedSurface) SurfaceFactory::BuildRectangularTrimmedSurface(IIfcRectangularTrimmedSurface^ ifcRectangularTrimmedSurface)
			{
				XSurfaceType surfaceType;
				Handle(Geom_Surface) basisSurface = BuildSurface(ifcRectangularTrimmedSurface->BasisSurface, surfaceType); //throws an exception with any failure

				Handle(Geom_RectangularTrimmedSurface) geomTrim = new  Geom_RectangularTrimmedSurface(basisSurface, ifcRectangularTrimmedSurface->U1,
					ifcRectangularTrimmedSurface->U2, ifcRectangularTrimmedSurface->V1, ifcRectangularTrimmedSurface->V2);
				return geomTrim;
			}

			Handle(Geom_BSplineSurface) SurfaceFactory::BuildBSplineSurfaceWithKnots(IIfcBSplineSurfaceWithKnots^ ifcBSplineSurfaceWithKnots)
			{
				auto ifcControlPoints = ifcBSplineSurfaceWithKnots->ControlPoints;
				if (ifcControlPoints->Count < 2)
					throw RaiseGeometryFactoryException("Incorrect number of poles for Bspline surface, it must be at least 2", ifcBSplineSurfaceWithKnots);

				TColgp_Array2OfPnt poles(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->UUpper + 1, 1, (Standard_Integer)ifcBSplineSurfaceWithKnots->VUpper + 1);

				for (int u = 0; u <= ifcBSplineSurfaceWithKnots->UUpper; u++)
				{
					auto uRow = ifcControlPoints[u];
					for (int v = 0; v <= ifcBSplineSurfaceWithKnots->VUpper; v++)
					{
						poles.SetValue(u + 1, v + 1, gp_Pnt(uRow[v].X, uRow[v].Y, uRow[v].Z));
					}
				}

				TColStd_Array1OfReal uknots(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->KnotUUpper);
				TColStd_Array1OfReal vknots(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->KnotVUpper);
				TColStd_Array1OfInteger uMultiplicities(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->KnotUUpper);
				TColStd_Array1OfInteger vMultiplicities(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->KnotVUpper);
				int i = 1;
				for each (double knot in ifcBSplineSurfaceWithKnots->UKnots)
				{
					uknots.SetValue(i, knot);
					i++;
				}
				i = 1;
				for each (double knot in ifcBSplineSurfaceWithKnots->VKnots)
				{
					vknots.SetValue(i, knot);
					i++;
				}

				i = 1;
				for each (int multiplicity in ifcBSplineSurfaceWithKnots->UMultiplicities)
				{
					uMultiplicities.SetValue(i, multiplicity);
					i++;
				}

				i = 1;
				for each (int multiplicity in ifcBSplineSurfaceWithKnots->VMultiplicities)
				{
					vMultiplicities.SetValue(i, multiplicity);
					i++;
				}

				Handle(Geom_BSplineSurface) hSurface = new Geom_BSplineSurface(poles, uknots, vknots, uMultiplicities, vMultiplicities, (Standard_Integer)ifcBSplineSurfaceWithKnots->UDegree, (Standard_Integer)ifcBSplineSurfaceWithKnots->VDegree);
				return hSurface;
			}

			Handle(Geom_BSplineSurface) SurfaceFactory::BuildRationalBSplineSurfaceWithKnots(IIfcRationalBSplineSurfaceWithKnots^ ifcRationalBSplineSurfaceWithKnots)
			{
				auto ifcControlPoints = ifcRationalBSplineSurfaceWithKnots->ControlPoints;
				if (ifcControlPoints->Count < 2)
					throw RaiseGeometryFactoryException("Incorrect number of poles for Bspline surface, it must be at least 2", ifcRationalBSplineSurfaceWithKnots);

				TColgp_Array2OfPnt poles(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->UUpper + 1, 1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->VUpper + 1);

				for (int u = 0; u <= ifcRationalBSplineSurfaceWithKnots->UUpper; u++)
				{
					auto uRow = ifcControlPoints[u];
					for (int v = 0; v <= ifcRationalBSplineSurfaceWithKnots->VUpper; v++)
					{
						poles.SetValue(u + 1, v + 1, gp_Pnt(uRow[v].X, uRow[v].Y, uRow[v].Z));
					}
				}
				auto ifcWeights = ifcRationalBSplineSurfaceWithKnots->Weights;
				TColStd_Array2OfReal weights(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->UUpper + 1, 1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->VUpper + 1);
				for (int u = 0; u <= ifcRationalBSplineSurfaceWithKnots->UUpper; u++)
				{
					List<Ifc4::MeasureResource::IfcReal>^ uRow = ifcWeights[u];
					for (int v = 0; v <= ifcRationalBSplineSurfaceWithKnots->VUpper; v++)
					{
						double r = uRow[v];
						weights.SetValue(u + 1, v + 1, r);
					}
				}


				TColStd_Array1OfReal uknots(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->KnotUUpper);
				TColStd_Array1OfReal vknots(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->KnotVUpper);
				TColStd_Array1OfInteger uMultiplicities(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->KnotUUpper);
				TColStd_Array1OfInteger vMultiplicities(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->KnotVUpper);
				int i = 1;
				for each (double knot in ifcRationalBSplineSurfaceWithKnots->UKnots)
				{
					uknots.SetValue(i, knot);
					i++;
				}
				i = 1;
				for each (double knot in ifcRationalBSplineSurfaceWithKnots->VKnots)
				{
					vknots.SetValue(i, knot);
					i++;
				}

				i = 1;
				for each (int multiplicity in ifcRationalBSplineSurfaceWithKnots->UMultiplicities)
				{
					uMultiplicities.SetValue(i, multiplicity);
					i++;
				}

				i = 1;
				for each (int multiplicity in ifcRationalBSplineSurfaceWithKnots->VMultiplicities)
				{
					vMultiplicities.SetValue(i, multiplicity);
					i++;
				}

				Handle(Geom_BSplineSurface) hSurface = new Geom_BSplineSurface(poles, weights, uknots, vknots, uMultiplicities, vMultiplicities, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->UDegree, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->VDegree);
				return hSurface;
			}

			Handle(Geom_CylindricalSurface) SurfaceFactory::BuildCylindricalSurface(IIfcCylindricalSurface^ ifcCylindricalSurface)
			{
				if (ifcCylindricalSurface->Radius <= 0)
					throw RaiseGeometryFactoryException("Radius for surface must be > 0", ifcCylindricalSurface);
				TopLoc_Location axLoc = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(ifcCylindricalSurface->Position);
				auto cylinderSurface = EXEC_NATIVE->BuildCylindricalSurface(ifcCylindricalSurface->Radius);
				if (cylinderSurface.IsNull())
					throw RaiseGeometryFactoryException("Failed to build cylinder surface, see logs for details", ifcCylindricalSurface);

				cylinderSurface->Transform(axLoc);
				return cylinderSurface;
			}

			Handle(Geom_SphericalSurface) SurfaceFactory::BuildSphericalSurface(IIfcSphericalSurface^ ifcSphericalSurface)
			{
				if (ifcSphericalSurface->Radius <= 0)
					throw RaiseGeometryFactoryException("Radius for surface must be > 0", ifcSphericalSurface);
				gp_Ax2 ax2;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement3d(ifcSphericalSurface->Position, ax2))
					throw RaiseGeometryFactoryException("Invalid axis for surface", ifcSphericalSurface);
				gp_Ax3 ax3(ax2);

				return new Geom_SphericalSurface(ax3, ifcSphericalSurface->Radius);
			}

			Handle(Geom_ToroidalSurface) SurfaceFactory::BuildToroidalSurface(IIfcToroidalSurface^ ifcToroidalSurface)
			{
				if (ifcToroidalSurface->MajorRadius < 0 || ifcToroidalSurface->MinorRadius < 0)
					throw RaiseGeometryFactoryException("Radius for surface must be >= 0", ifcToroidalSurface);
				if (ifcToroidalSurface->MinorRadius.Equals(0))
					throw RaiseGeometryFactoryException("Circle Radius for surface must be > 0", ifcToroidalSurface);
				gp_Ax2 ax2;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement3d(ifcToroidalSurface->Position, ax2))
					throw RaiseGeometryFactoryException("Invalid axis for surface", ifcToroidalSurface);
				gp_Ax3 ax3(ax2);
				return new Geom_ToroidalSurface(ax3, ifcToroidalSurface->MajorRadius, ifcToroidalSurface->MinorRadius);
			}

#pragma endregion
		}
	}
}
