#include "ShellFactory.h"
#include "FaceFactory.h"
#include "GeometryFactory.h"
#include <vector>
#include <unordered_map>
using namespace System::Collections::Generic;
using namespace System::Linq;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			TopoDS_Shell ShellFactory::BuildClosedShell(IIfcClosedShell^ closedShell, CheckClosedStatus& isCheckedClosed)
			{
				return BuildConnectedFaceSet(closedShell, isCheckedClosed);
			}

			TopoDS_Shell ShellFactory::BuildConnectedFaceSet(IIfcConnectedFaceSet^ faceSet, CheckClosedStatus& isCheckedClosed)
			{
				if (faceSet->CfsFaces->Count == 0) return TopoDS_Shell();
				if (dynamic_cast<IIfcFaceSurface^>(Enumerable::First(faceSet->CfsFaces)))
					return ShellFactory::BuildConnectedFaceSurfaceSet(faceSet, isCheckedClosed);
				else //build IfcFace set
				{
					if (!dynamic_cast<IIfcFace^>(Enumerable::First(faceSet->CfsFaces)))
						throw RaiseGeometryFactoryException("IfcConnectedFaceSet must comprises faces of either IfcFace, IfcFaceSurface or IfcAdvancedFace", faceSet);
					//process into native structure
					std::vector<std::vector<std::vector<int>>> faceMesh;
					std::unordered_map<int, gp_XYZ> points;

					for each (IIfcFace ^ face in  faceSet->CfsFaces)
					{
						std::vector<std::vector<int>> faceLoops;

						for each (IIfcFaceBound ^ bound in face->Bounds) //build all the loops
						{
							IIfcPolyLoop^ polyloop = dynamic_cast<IIfcPolyLoop^>(bound->Bound);
							if (polyloop != nullptr)
							{
								IEnumerable<IIfcCartesianPoint^>^ polygon = polyloop->Polygon;
								if (!bound->Orientation) polygon = Enumerable::Reverse(polyloop->Polygon);
								std::vector<int> loop;
								for each (IIfcCartesianPoint ^ cp in Enumerable::Concat(polyloop->Polygon, Enumerable::Take(polyloop->Polygon, 1)))
								{
									points[cp->EntityLabel] = GEOMETRY_FACTORY->BuildXYZ(cp);
									loop.push_back(cp->EntityLabel);
								}
								if (loop.size() < 4) //its not a min of a closed triangle, fourth point is first point repeated for closure
									LogDebug(polyloop, "Bound is not a valid boundary, it has less than 3 points");
								else
									faceLoops.push_back(loop);
							}
						}
						faceMesh.push_back(faceLoops);
					}
					TopoDS_Shell shell = OccHandle().BuildConnectedFaceSet(faceMesh, points, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
					if (shell.IsNull())
						throw RaiseGeometryFactoryException("Failed to build connected face set", faceSet);
					isCheckedClosed = NotChecked;
					return shell;
				}
			}

			TopoDS_Shell ShellFactory::BuildConnectedFaceSurfaceSet(IIfcConnectedFaceSet^ faceSet, CheckClosedStatus& isCheckedClosed)
			{
				if (faceSet->CfsFaces->Count == 0) return TopoDS_Shell();
				if (!dynamic_cast<IIfcFaceSurface^>(Enumerable::First(faceSet->CfsFaces)))
					return BuildConnectedFaceSet(faceSet, isCheckedClosed);
				TopTools_DataMapOfIntegerShape edges;
				TopTools_DataMapOfIntegerShape vertices;
				TopTools_DataMapOfIntegerShape faces;
				TopoDS_Shell shell;
				BRep_Builder builder;
				builder.MakeShell(shell);
				for each (auto ifcFaceSurface in Enumerable::Cast<IIfcFaceSurface^>(faceSet->CfsFaces))
				{
					TopoDS_Face face = FACE_FACTORY->BuildFaceSurface(ifcFaceSurface, edges, vertices);
					faces.Bind(ifcFaceSurface->EntityLabel, face);
					builder.Add(shell, face);
					//System::String^ brep = _modelService->GetBrep(shell);
				}
				isCheckedClosed = NotChecked;

				return shell;
			}

			TopoDS_Shell ShellFactory::BuildPolygonalFaceSet(IIfcPolygonalFaceSet^ faceSet, CheckClosedStatus& isCheckedClosed)
			{
				if (faceSet->Faces->Count == 0) return TopoDS_Shell();
				std::unordered_map<int, gp_XYZ> points;
				std::vector<std::vector<std::vector<int>>> faceMesh;
				int i = 1;
				for each (auto coords in faceSet->Coordinates->CoordList)
				{
					points[i++] = gp_XYZ(coords[0], coords[1], coords[2]);
				}

				
				for each (auto face in faceSet->Faces)
				{
					std::vector<std::vector<int>> faceLoops;
					
					std::vector<int> outerLoop;
					for each (auto coordIndex in Enumerable::Concat(face->CoordIndex, Enumerable::Take(face->CoordIndex, 1))) //repeat first point at end to close the loop
					{
						outerLoop.push_back((int)coordIndex);
					}
					faceLoops.push_back(outerLoop); //outer loop
					
					IIfcIndexedPolygonalFaceWithVoids^  faceWithVoids = dynamic_cast<IIfcIndexedPolygonalFaceWithVoids^>(face);
					if (faceWithVoids != nullptr)
					{
						for each (auto faceVoid in faceWithVoids->InnerCoordIndices)
						{
							std::vector<int> innerLoop;
							for each (auto coordIndex in Enumerable::Concat(faceVoid, Enumerable::Take(faceVoid, 1))) //repeat first point at end to close the loop
							{
								innerLoop.push_back((int)coordIndex);
							}
							faceLoops.push_back(innerLoop); //outer loop
						}
					}
					faceMesh.push_back(faceLoops);
				}

				TopoDS_Shell shell = OccHandle().BuildConnectedFaceSet(faceMesh, points, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
				if (shell.IsNull())
					throw RaiseGeometryFactoryException("Failed to build PolygonalFaceSet", faceSet);
				isCheckedClosed = NotChecked;
				return shell;
			}
		}
	}
}

