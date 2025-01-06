#include "ShellFactory.h"
#include "FaceFactory.h"
#include "SurfaceFactory.h"
#include "GeometryFactory.h"
#include "BooleanFactory.h"
#include <vector>
#include <unordered_map>
#include <ShapeFix_Shape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TColgp_SequenceOfAx1.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepCheck_Shell.hxx>

using namespace System::Collections::Generic;
using namespace System::Linq;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			TopoDS_Shape ShellFactory::BuildClosedShell(IIfcClosedShell^ closedShell, bool& isFixed)
			{
				TopoDS_Shape shape = BuildConnectedFaceSet(closedShell, isFixed);
				if (shape.IsNull())
				{
					LogDebug(closedShell, "Closed shell is empty");
					return shape;
				}
				//since we have planar faces only then any valid solid must have at least 4 faces (pyramid), we will dispose of any shells that have less than four planar faces
				//upgrade to solids
				//if a compound is returned we have more than one shell
				ShapeFix_Solid sfs;
				if (shape.ShapeType() == TopAbs_COMPOUND)
				{
					//any shells in here will have been fixed
					BRep_Builder b;
					TopoDS_Compound solidCompound;
					b.MakeCompound(solidCompound);

					for (TopoDS_Iterator childIterator(shape); childIterator.More(); childIterator.Next())
						if (childIterator.Value().ShapeType() == TopAbs_SHELL)
						{
							const TopoDS_Shell& shell = TopoDS::Shell(childIterator.Value());
							if (shell.NbChildren() < 4)
								LogDebug(closedShell, "A Closed shell is not a solid, it has less than 4 valid faces and has been ignored");
							else
								b.Add(solidCompound, sfs.SolidFromShell(shell));
						}
					return solidCompound;
				}
				else if (shape.ShapeType() == TopAbs_SHELL)//it will be a shell
				{
					const TopoDS_Shell& shell = TopoDS::Shell(shape);
					if (shell.NbChildren() < 4)
						LogDebug(closedShell, "Closed shell is not a solid, it has less than 4 valid faces and has been ignored");
					//it could be a shell of many solids as IFC permits this
					//only check topology as geometry should be correct
					if (ModelGeometryService->UpgradeFaceSets) //this allows better performance if set to false
					{
						BRepCheck_Analyzer topologyChecker(shell, false, false, false);
						if (topologyChecker.IsValid())
							return sfs.SolidFromShell(shell);
						else //most likely it is a collection of solids inside a shell
						{
							ShapeFix_Shape shapeFixer(shell);
							if (shapeFixer.Perform())
							{
								//ensure that each separate shell is a solid
								BRep_Builder b;
								TopoDS_Compound solidCompound;
								b.MakeCompound(solidCompound);
								for (TopoDS_Iterator exp(shapeFixer.Shape()); exp.More(); exp.Next())
								{
									if (exp.Value().ShapeType() == TopAbs_SHELL)
										b.Add(solidCompound, sfs.SolidFromShell(TopoDS::Shell(exp.Value())));
									else if (exp.Value().ShapeType() == TopAbs_SOLID)
										b.Add(solidCompound, TopoDS::Solid(exp.Value()));
								}
								if (solidCompound.NbChildren() == 1) //trim off the compound if we ony have one solid
								{
									return BOOLEAN_FACTORY->EXEC_NATIVE->TrimTopology(solidCompound);
								}
								else
									return solidCompound;
							}
						}
					}
					return sfs.SolidFromShell(shell);
				}
				else
					throw RaiseGeometryFactoryException("Failed to build closed shell", closedShell);
			}

			/// <summary>
			/// Builds a connected face set where each face is planar, use BuildConnectedAdvancedFaceSet for advanced faces
			/// </summary>
			/// <param name="faceSet"></param>
			/// <param name="isFixed"></param>
			/// <returns></returns>
			TopoDS_Shape ShellFactory::BuildConnectedFaceSet(IIfcConnectedFaceSet^ faceSet, bool& isFixed)
			{
				if (faceSet->CfsFaces->Count == 0) return TopoDS_Shell();
				if (dynamic_cast<IIfcAdvancedFace^>(Enumerable::First(faceSet->CfsFaces)))
					return ShellFactory::BuildConnectedAdvancedFaceSet(faceSet, isFixed);

				//build as IfcFace set
				if (!dynamic_cast<IIfcFace^>(Enumerable::First(faceSet->CfsFaces)))
					throw RaiseGeometryFactoryException("IfcConnectedFaceSet must comprises faces of either IfcFace, IfcFaceSurface or IfcAdvancedFace", faceSet);
				//process into native structure
				std::vector<std::vector<std::vector<int>>> faceMesh;
				std::unordered_map<int, gp_XYZ> points;
				TColgp_SequenceOfAx1 planAxes;
				std::vector<int> planeIndices; planeIndices.reserve(faceSet->CfsFaces->Count);
				for each (IIfcFace ^ face in  faceSet->CfsFaces)
				{
					std::vector<std::vector<int>> faceLoops;
					auto faceSurface = dynamic_cast<IIfcFaceSurface^>(face);
					int planeIndex = 0;
					if (faceSurface != nullptr)
					{
						auto facePlane = dynamic_cast<IIfcPlane^>(faceSurface->FaceSurface);
						if (facePlane != nullptr)
						{
							auto plane = SURFACE_FACTORY->BuildPlane(facePlane, false);
							planAxes.Append(faceSurface->SameSense ? plane->Axis() : plane->Axis().Reversed());
							planeIndex = planAxes.Size();
						}
					}
					planeIndices.push_back(planeIndex);
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
				bool needsFixing;
				TopoDS_Shell shell = EXEC_NATIVE->BuildConnectedFaceSet(faceMesh, points, planeIndices, planAxes, ModelGeometryService->Precision, ModelGeometryService->MinimumGap, needsFixing);
				if (shell.IsNull())
					throw RaiseGeometryFactoryException("Failed to build connected face set", faceSet);
				if (needsFixing)
				{
					LogDebug(faceSet, "Attempting to fix errors in faceset definition");
					return FixShell(shell, faceSet, isFixed);
				}
				return shell;
			}

			TopoDS_Shape ShellFactory::FixShell(TopoDS_Shell& shell, IPersistEntity^ entity, bool& isFixed)
			{
				auto fixed = EXEC_NATIVE->FixShell(shell, isFixed);
				if (isFixed) {
					return fixed;
				}
				LogWarning(entity, "Connected face set has definition errors that could not be fixed");
				return shell;
			}

			TopoDS_Shape ShellFactory::BuildConnectedAdvancedFaceSet(IIfcConnectedFaceSet^ faceSet, bool& isFixed)
			{
				if (faceSet->CfsFaces->Count == 0) return TopoDS_Shell();
				if (!dynamic_cast<IIfcFaceSurface^>(Enumerable::First(faceSet->CfsFaces)))
					return BuildConnectedFaceSet(faceSet, isFixed);
				TopTools_DataMapOfIntegerShape edges;
				TopTools_DataMapOfIntegerShape vertices;
				TopTools_DataMapOfIntegerShape faces;
				TopoDS_Shell shell;
				BRep_Builder builder;
				builder.MakeShell(shell);
				for each (auto ifcFaceSurface in Enumerable::Cast<IIfcAdvancedFace^>(faceSet->CfsFaces))
				{
					TopoDS_Face face = FACE_FACTORY->BuildAdvancedFace(ifcFaceSurface, edges, vertices);
					if (!face.IsNull()) //skip face it it is empty
					{
						faces.Bind(ifcFaceSurface->EntityLabel, face);
						builder.Add(shell, face);
					}
					//System::String^ brep = _modelService->GetBrep(shell);
				}

				return shell;
			}

			TopoDS_Shape ShellFactory::BuildPolygonalFaceSet(IIfcPolygonalFaceSet^ faceSet, bool& isFixed)
			{
				if (faceSet->Faces->Count == 0) return TopoDS_Shell();
				std::unordered_map<int, gp_XYZ> points;
				std::vector<std::vector<std::vector<int>>> faceMesh;
				int i = 1;
				for each (auto coords in faceSet->Coordinates->CoordList)
				{
					points[i++] = gp_XYZ(coords[0], coords[1], coords[2]);
				}
				TColgp_SequenceOfAx1 planAxes;
				std::vector<int> planeIndices; planeIndices.reserve(faceSet->Faces->Count);
				for each (auto face in faceSet->Faces)
				{
					planeIndices.push_back(0);
					std::vector<std::vector<int>> faceLoops;

					std::vector<int> outerLoop;
					for each (auto coordIndex in Enumerable::Concat(face->CoordIndex, Enumerable::Take(face->CoordIndex, 1))) //repeat first point at end to close the loop
					{
						outerLoop.push_back((int)coordIndex);
					}
					faceLoops.push_back(outerLoop); //outer loop

					IIfcIndexedPolygonalFaceWithVoids^ faceWithVoids = dynamic_cast<IIfcIndexedPolygonalFaceWithVoids^>(face);
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
				bool needsFixing;
				TopoDS_Shell shell = EXEC_NATIVE->BuildConnectedFaceSet(faceMesh, points, planeIndices, planAxes, ModelGeometryService->Precision, ModelGeometryService->MinimumGap, needsFixing);
				if (shell.IsNull())
					throw RaiseGeometryFactoryException("Failed to build PolygonalFaceSet", faceSet);
				if (needsFixing)
				{
					LogDebug(faceSet, "Attempting to fix errors in faceset definition");
					return FixShell(shell, faceSet, isFixed);
				}
				return shell;

			}

			TopoDS_Shape ShellFactory::BuildFaceBaseSurfaceModel(IIfcFaceBasedSurfaceModel^ fbsm, bool& isFixed)
			{
				BRep_Builder b;
				TopoDS_Compound  faceSetShape;
				b.MakeCompound(faceSetShape);
				for each (auto faceSet in fbsm->FbsmFaces)
				{
					auto shape = BuildConnectedFaceSet(faceSet, isFixed);
					if (!shape.IsNull())
						b.Add(faceSetShape, shape);
				}
				return faceSetShape;
			}

			TopoDS_Shape ShellFactory::BuildShellBaseSurfaceModel(IIfcShellBasedSurfaceModel^ sbsm, bool& isFixed)
			{
				BRep_Builder b;
				TopoDS_Compound  faceSetShape;
				b.MakeCompound(faceSetShape);
				for each (auto ifcShell in sbsm->SbsmBoundary)
				{
					IIfcClosedShell^ closedShell = dynamic_cast<IIfcClosedShell^>(ifcShell);
					IIfcOpenShell^ openShell = dynamic_cast<IIfcOpenShell^>(ifcShell);
					bool isFixed;
					if (openShell != nullptr)
					{
						auto shell = BuildConnectedFaceSet(openShell, isFixed);
						if (!shell.IsNull())
							b.Add(faceSetShape, shell);
					}
					else if (closedShell != nullptr)
					{
						auto shell = BuildConnectedFaceSet(closedShell, isFixed);
						if (!shell.IsNull())
							b.Add(faceSetShape, shell);
					}
				}
				return EXEC_NATIVE->TrimTopology(faceSetShape);
			}
		}
	}
}

