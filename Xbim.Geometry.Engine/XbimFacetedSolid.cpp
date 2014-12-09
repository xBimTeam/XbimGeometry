#ifdef USE_CARVE_CSG

#include "XbimFacetedSolid.h"
#include "XbimWireSet.h"
#include "XbimPoint3DWithTolerance.h"
#include "XbimGeometryCreator.h"
#include "XbimVertexSet.h"
#include "XbimEdgeSet.h"
#include "XbimFaceSet.h"
#include "XbimShellSet.h"
#include "XbimFacetedShell.h"
#include "XbimPolygonalFace.h"
#include "XbimLinearEdge.h"
#include "XbimSolidSet.h"

#pragma region Occ headers
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <BRep_Tool.hxx>  
#include <BRepBuilderAPI_Sewing.hxx>  
#include <Geom_Plane.hxx>  
#include <gp_Pln.hxx>  
#include <BRepTools.hxx>
#include <TopExp.hxx> 

#pragma endregion

#pragma region Carve CSG Headers

#include <carve\Triangulator.hpp>
#include <carve\Input.hpp>
#include <carve\mesh_simplify.hpp>
#pragma endregion

using namespace Xbim::Common;
using namespace System::Threading;
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimFacetedSolid::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(pMeshSet, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (meshset_t*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}


#pragma region Constructors
		XbimFacetedSolid::XbimFacetedSolid()
		{
		}

		XbimFacetedSolid::XbimFacetedSolid(meshset_t* mesh)
		{
			pMeshSet = IntPtr(mesh);
		}

		//Converts an OCC type solid to a faceted solid
		XbimFacetedSolid::XbimFacetedSolid(XbimSolid^ solid, double tolerance, double deflection)
		{
			Init(solid, tolerance, deflection, 0.5, false);
		}

		//Converts an OCC type solid to a faceted solid
		XbimFacetedSolid::XbimFacetedSolid(IXbimSolid^ solid, double tolerance, double deflection)
		{
			XbimSolid^ xSolid = dynamic_cast<XbimSolid^>(solid);
			if (xSolid == nullptr)
				throw gcnew Exception("Only  Xbim..... objects can be converted to faceted shells");
			Init(xSolid, tolerance, deflection, 0.5, false);
		}

		XbimFacetedSolid::XbimFacetedSolid(IXbimSolid^ solid, double tolerance, double deflection, bool triangulate)
		{
			XbimSolid^ xSolid = dynamic_cast<XbimSolid^>(solid);
			if (xSolid == nullptr)
				throw gcnew Exception("Only  Xbim..... objects can be converted to faceted shells");
			Init(xSolid, tolerance, deflection, 0.5, triangulate);
		}


		//Converts an OCC type solid to a faceted solid
		XbimFacetedSolid::XbimFacetedSolid(XbimSolid^ solid, double tolerance)
		{
			Init(solid, tolerance, 1.0, 0.5, false);
		}

		XbimFacetedSolid::XbimFacetedSolid(XbimSolid^ solid, double tolerance, double deflection, double angle)
		{
			Init(solid, tolerance, deflection, angle, false);
		}

		XbimFacetedSolid::XbimFacetedSolid(IXbimSolid^ solid, double tolerance, double deflection, double angle)
		{
			XbimSolid^ xSolid = dynamic_cast<XbimSolid^>(solid);
			if (xSolid == nullptr)
				throw gcnew Exception("Only  Xbim..... objects can be converted to faceted shells");
			Init(xSolid, tolerance, deflection, angle, false);
		}

		XbimFacetedSolid::XbimFacetedSolid(IXbimSolid^ solid, double tolerance, double deflection, double angle, bool triangulate)
		{
			XbimSolid^ xSolid = dynamic_cast<XbimSolid^>(solid);
			if (xSolid == nullptr)
				throw gcnew Exception("Only  Xbim..... objects can be converted to faceted shells");
			Init(xSolid, tolerance, deflection, angle, triangulate);
		}
		
		XbimFacetedSolid::XbimFacetedSolid(IfcBooleanClippingResult^ clip)
		{
			Init(clip);
		}
		
		XbimFacetedSolid::XbimFacetedSolid(XbimShell^ shell)
		{
			throw gcnew NotImplementedException("Constructor from face not implemented");
		}

		XbimFacetedSolid::XbimFacetedSolid(XbimFace^ face)
		{
			throw gcnew NotImplementedException("Constructor from face not implemented");
		}
#pragma endregion

		///Returns the pointer to the facet mesh data, it is the responsibility of the caller to delete this when not required
		///This faceted solid is no longer valid after this call;
		IntPtr XbimFacetedSolid::Nullify()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(pMeshSet, IntPtr(0)); //swap out
			return temp;
		}

#pragma region Initialisation

		void XbimFacetedSolid::Init(IfcBooleanClippingResult^ clip)
		{
			XbimModelFactors^ mf = clip->ModelOf->ModelFactors;
			IfcBooleanOperand^ fOp = clip->FirstOperand;
			IfcBooleanOperand^ sOp = clip->SecondOperand;
			IXbimSolid^ left ;
			IXbimSolid^ right;
			if (dynamic_cast<IfcBooleanClippingResult^>(fOp))
				left = gcnew XbimFacetedSolid((IfcBooleanClippingResult^)fOp);
			else
				left = gcnew XbimSolid(fOp);
			if (dynamic_cast<IfcBooleanClippingResult^>(sOp))
				right = gcnew XbimFacetedSolid((IfcBooleanClippingResult^)sOp);
			else
				right = gcnew XbimSolid(sOp);
			
			if (!left->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS006: IfcBooleanResult #{0} with invalid first operand", clip->EntityLabel);
				return;
			}

			if (!right->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS007: IfcBooleanResult #{0} with invalid second operand", clip->EntityLabel);
				XbimFacetedSolid^ fs = dynamic_cast<XbimFacetedSolid^>(left);
				if (fs==nullptr)
					fs = gcnew XbimFacetedSolid(left, mf->Precision, mf->DeflectionTolerance, mf->DeflectionAngle);
				pMeshSet = fs->Nullify();
				tolerance = fs->Tolerance;
				return;
			}

			if (left->IsPolyhedron && right->IsPolyhedron) //if they are both faceted solids convert them to use carve
			{
				XbimFacetedSolid^ fs = dynamic_cast<XbimFacetedSolid^>(left);
				if (fs == nullptr)
					fs = gcnew XbimFacetedSolid(left, mf->Precision, mf->DeflectionTolerance, mf->DeflectionAngle);
				left = fs;
				fs = dynamic_cast<XbimFacetedSolid^>(right);
				if (fs == nullptr)
					fs = gcnew XbimFacetedSolid(right, mf->Precision, mf->DeflectionTolerance, mf->DeflectionAngle);
				right = fs;
			}
			IXbimGeometryObject^ result;
			try
			{
				switch (clip->Operator)
				{
				case IfcBooleanOperator::Union:
					result = left->Union(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::Intersection:
					result = left->Intersection(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::Difference:
					result = left->Cut(right, mf->PrecisionBoolean);
					break;
				}
			}
			catch (Exception^ xbimE)
			{
				XbimGeometryCreator::logger->ErrorFormat("ES001: Error performing boolean operation for entity #{0}={1}\n{2}. The operation has been ignored", clip->EntityLabel, clip->GetType()->Name, xbimE->Message);
				XbimFacetedSolid^ fs = dynamic_cast<XbimFacetedSolid^>(left);
				if (fs == nullptr)
					fs = gcnew XbimFacetedSolid(left, mf->Precision, mf->DeflectionTolerance, mf->DeflectionAngle);
				pMeshSet = fs->Nullify();
				tolerance = fs->Tolerance;
				return;
			}

			XbimSolidSet^ xbimSolidSet = dynamic_cast<XbimSolidSet^>(result);
			if (xbimSolidSet == nullptr || xbimSolidSet->First == nullptr)
			{
				XbimGeometryCreator::logger->ErrorFormat("ES002: Error performing boolean operation for entity #{0}={1}. The operation has been ignored", clip->EntityLabel, clip->GetType()->Name);
				XbimFacetedSolid^ fs = dynamic_cast<XbimFacetedSolid^>(left);
				if (fs == nullptr)
					fs = gcnew XbimFacetedSolid(left, mf->Precision, mf->DeflectionTolerance, mf->DeflectionAngle);
				pMeshSet = fs->Nullify();
				tolerance = fs->Tolerance;
			}
			else 
			{
				
				XbimFacetedSolid^ fs = dynamic_cast<XbimFacetedSolid^>(xbimSolidSet->First);
				if (fs == nullptr)
					fs = gcnew XbimFacetedSolid(xbimSolidSet->First, mf->Precision, mf->DeflectionTolerance, mf->DeflectionAngle);
				pMeshSet = fs->Nullify();
				tolerance = fs->Tolerance;
			}
		}


		void XbimFacetedSolid::InitPolyhedron(XbimSolid^ solid, double tolerance)
		{
			//create the Vertex Map to hold unique vertices and a store of the carve vertices
			std::vector<vertex_t> vertices;
			vertices.reserve(solid->Vertices->Count); //good for all faceted objects will need to grow further where curved surfaces are converted to facetations

			Dictionary<XbimPoint3DWithTolerance^, size_t>^ pointMap = gcnew Dictionary<XbimPoint3DWithTolerance^, size_t>();
			//Create a list to hold the face loops for each face, loops are indexes in to the vertex list
			std::vector<std::vector<size_t>> faceLoops;

			for each (XbimFace^ face in solid->Faces)
			{
				std::vector<size_t> faceLoop; //get a new face
				for each (XbimEdge^ edge in face->OuterBound->Edges)
				{
					gp_Pnt p = edge->IsReversed ? BRep_Tool::Pnt(TopExp::LastVertex(edge, Standard_False)) : BRep_Tool::Pnt(TopExp::FirstVertex(edge, Standard_False));
					size_t index;
					XbimPoint3DWithTolerance^ pt = gcnew XbimPoint3DWithTolerance(p.X(), p.Y(), p.Z(), tolerance);
					if (!pointMap->TryGetValue(pt, index))
					{
						index = vertices.size();
						pointMap->Add(pt, index);
						vertices.push_back(carve::geom::VECTOR(pt->X, pt->Y, pt->Z));
					}
					if (std::find(faceLoop.begin(), faceLoop.end(), index) == faceLoop.end()) //skip any point we have just added
						faceLoop.push_back(index);

				}

				if (faceLoop.size() > 2) //don't add the loop if it has just one edge
				{
					//now we have to do any inner bounds
					XbimWireSet^ innerBounds = (XbimWireSet^)face->InnerBounds;
					int innerBoundCount = innerBounds->Count;
					if (innerBoundCount == 0)
						faceLoops.push_back(faceLoop);
					else
					{
						std::vector<std::vector<size_t>> holes;
						holes.reserve(innerBoundCount);
						std::vector<std::vector<carve::geom2d::P2> > projected_poly;

						projected_poly.resize(innerBoundCount + 1);
						projected_poly[0].reserve(faceLoop.size());
						//create a face that fits the outer bound to project points on to
						std::vector<vertex_t *> v;
						size_t sizeOuter = faceLoop.size();
						v.reserve(sizeOuter);
						for (size_t i = 0; i < sizeOuter; ++i)
							v.push_back(&vertices[faceLoop[i]]);
						face_t projectionFace(v.begin(), v.end());
						//project all the points of the outerbound onto the 2D face
						for (size_t j = 0; j < faceLoop.size(); ++j)
							projected_poly[0].push_back(projectionFace.project(vertices[faceLoop[j]].v));

						//project the holes
						int boundIndex = 0;
						for each (XbimWire^ innerBound in face->InnerBounds)
						{
							holes.push_back(std::vector<size_t>());
							std::vector<size_t> & holeLoop = holes.back();
							int pointCount = innerBound->Vertices->Count;
							holeLoop.reserve(pointCount);
							projected_poly[boundIndex + 1].reserve(pointCount);
							for each (XbimEdge^ edge in innerBound->Edges)
							{
								gp_Pnt p = edge->IsReversed ? BRep_Tool::Pnt(TopExp::LastVertex(edge, Standard_False)) : BRep_Tool::Pnt(TopExp::FirstVertex(edge, Standard_False));

								size_t index;
								XbimPoint3DWithTolerance^ pt = gcnew XbimPoint3DWithTolerance(p.X(), p.Y(), p.Z(), tolerance);
								if (!pointMap->TryGetValue(pt, index))
								{
									index = vertices.size();
									pointMap->Add(pt, index);
									vertices.push_back(carve::geom::VECTOR(pt->X, pt->Y, pt->Z));
								}

								if (std::find(holeLoop.begin(), holeLoop.end(), index) == holeLoop.end())
								{
									holeLoop.push_back(index); //don't add the same point twice
									projected_poly[boundIndex + 1].push_back(projectionFace.project(vertices[index].v));
								}
							}
							boundIndex++;
						}
						std::vector<std::pair<size_t, size_t> > result = carve::triangulate::incorporateHolesIntoPolygon(projected_poly);
						faceLoops.push_back(std::vector<size_t>());
						std::vector<size_t> &out = faceLoops.back();
						out.reserve(result.size());
						for (size_t j = 0; j < result.size(); ++j)
						{
							if (result[j].first == 0)
								out.push_back(faceLoop[result[j].second]);
							else
								out.push_back(holes[result[j].first - 1][result[j].second]);
						}
					}
				}
			}
			//convert the faces and vertices to a mesh set
			std::vector<face_t *> faceList;
			faceList.reserve(faceLoops.size());
			for (size_t i = 0; i < faceLoops.size(); ++i)
			{
				std::vector<vertex_t *> vf;
				size_t loopSize = faceLoops[i].size();
				vf.reserve(loopSize);
				for (size_t j = 0; j < loopSize; ++j)
				{
					vf.push_back(&vertices[faceLoops[i][j]]);
				}
				faceList.push_back(new face_t(vf.begin(), vf.end()));
			}
			std::vector<mesh_t*> meshes;
			mesh_t::create(faceList.begin(), faceList.end(), meshes, carve::mesh::MeshOptions().avoid_cavities(true), tolerance*tolerance, false);
			pMeshSet = IntPtr(new meshset_t(vertices, meshes));
		}

		void XbimFacetedSolid::Init(XbimSolid^ solid, double tol, double deflection, double angle, bool triangulate)
		{
			this->tolerance = tol;
			if (!triangulate && solid->IsPolyhedron) return InitPolyhedron(solid, tolerance);
			//Assumes the solid is topologically valid
			//checks each face, if the face is planar then the whole face including holes is added as a faceted face, curved edges are facetted to deflection and angle
			//if the face has curves it is triangulated and added as compound faces
			//precision is used to merge points which are within precision distance of each other
			//first make sure the solid is triangulated
			Monitor::Enter(solid);
			try
			{
				BRepMesh_IncrementalMesh incrementalMesh(solid, deflection, Standard_False, angle); //triangulate the first time				
			}
			finally
			{
				Monitor::Exit(solid);
			}

			//create the Vertex Map to hold unique vertices and a store of the carve vertices
			std::vector<vertex_t> vertices;
			vertices.reserve(solid->Vertices->Count); //good for all faceted objects will need to grow further where curved surfaces are converted to facetations

			Dictionary<XbimPoint3DWithTolerance^, size_t>^ pointMap = gcnew Dictionary<XbimPoint3DWithTolerance^, size_t>();
			//Create a list to hold the face loops for each face, loops are indexes in to the vertex list
			std::vector<std::vector<size_t>> faceLoops;

			for each (XbimFace^ face in solid->Faces)
			{
				TopLoc_Location loc;
				const Handle_Poly_Triangulation& mesh = BRep_Tool::Triangulation(face, loc);
				if (mesh.IsNull()) //this can happen with half spaces and triangles that are near to a line
				{
					continue;
				}

				
				const TColgp_Array1OfPnt & nodes = mesh->Nodes();
				//If the face is planar we only need to write out the bounding edges
				if (!triangulate && face->IsPlanar)
				{

					std::vector<size_t> faceLoop; //get a new face
					
					for each (XbimEdge^ edge in face->OuterBound->Edges)
					{
						Handle_Poly_PolygonOnTriangulation edgeMesh = BRep_Tool::PolygonOnTriangulation(edge, mesh, loc);
						if (edgeMesh.IsNull())
							continue;
						bool reverse = edge->IsReversed;
						int numNodes = edgeMesh->NbNodes(); //nb we skip the last node
						for (Standard_Integer i = reverse ? numNodes : 1; reverse ? i > 1:i < numNodes; reverse ? i-- : i++)
						{
							gp_XYZ p = nodes.Value(edgeMesh->Nodes().Value(i)).XYZ();
							loc.Transformation().Transforms(p);
							size_t index;
							XbimPoint3DWithTolerance^ pt = gcnew XbimPoint3DWithTolerance(p.X(), p.Y(), p.Z(), tolerance);
							if (!pointMap->TryGetValue(pt, index))
							{
								index = vertices.size();
								pointMap->Add(pt, index);
								vertices.push_back(carve::geom::VECTOR(pt->X, pt->Y, pt->Z));
							}
							if (std::find(faceLoop.begin(), faceLoop.end(), index)==faceLoop.end()) //skip any point we have just added
								faceLoop.push_back(index);
							/*else
								XbimGeometryCreator::logger->InfoFormat("Duplicate vertex found and has been ignored");*/
						}
					}
					
					if (faceLoop.size() > 2) //don't add the loop if it has just one edge
					{
						//now we have to do any inner bounds
						XbimWireSet^ innerBounds = (XbimWireSet^)face->InnerBounds;
						int innerBoundCount = innerBounds->Count;
						if (innerBoundCount == 0)
							faceLoops.push_back(faceLoop);
						else
						{
							std::vector<std::vector<size_t>> holes;
							holes.reserve(innerBoundCount);
							std::vector<std::vector<carve::geom2d::P2> > projected_poly;

							projected_poly.resize(innerBoundCount + 1);
							projected_poly[0].reserve(faceLoop.size());
							//create a face that fits the outer bound to project points on to
							std::vector<vertex_t *> v;
							size_t sizeOuter = faceLoop.size();
							v.reserve(sizeOuter);
							for (size_t i = 0; i < sizeOuter; ++i)
								v.push_back(&vertices[faceLoop[i]]);
							face_t projectionFace(v.begin(), v.end());
							//project all the points of the outerbound onto the 2D face
							for (size_t j = 0; j < faceLoop.size(); ++j)
								projected_poly[0].push_back(projectionFace.project(vertices[faceLoop[j]].v));

							//project the holes
							int boundIndex = 0;
							for each (XbimWire^ innerBound in face->InnerBounds)
							{
								holes.push_back(std::vector<size_t>());
								std::vector<size_t> & holeLoop = holes.back();
								int pointCount = innerBound->Vertices->Count;
								holeLoop.reserve(pointCount);
								projected_poly[boundIndex + 1].reserve(pointCount);
								for each (XbimEdge^ edge in innerBound->Edges)
								{
									Handle_Poly_PolygonOnTriangulation edgeMesh = BRep_Tool::PolygonOnTriangulation(edge, mesh, loc);
									if (edgeMesh.IsNull())
										continue;
									bool reverse = edge->IsReversed;
									int numNodes = edgeMesh->NbNodes(); //nb we skip the last node
									for (Standard_Integer i = reverse ? numNodes : 1; reverse ? i > 1:i < numNodes; reverse ? i-- : i++)
									{
										gp_XYZ p = nodes.Value(edgeMesh->Nodes().Value(i)).XYZ();
										loc.Transformation().Transforms(p);
										size_t index;
										XbimPoint3DWithTolerance^ pt = gcnew XbimPoint3DWithTolerance(p.X(), p.Y(), p.Z(), tolerance);
										if (!pointMap->TryGetValue(pt, index))
										{
											index = vertices.size();
											pointMap->Add(pt, index);
											vertices.push_back(carve::geom::VECTOR(pt->X, pt->Y, pt->Z));
										}

										if (std::find(holeLoop.begin(), holeLoop.end(), index) == holeLoop.end())
										{
											holeLoop.push_back(index); //don't add the same point twice
											projected_poly[boundIndex + 1].push_back(projectionFace.project(vertices[index].v));
										}
										/*else
											XbimGeometryCreator::logger->InfoFormat("Duplicate vertex found and has been ignored");*/
									}
								}
								boundIndex++;
							}
							std::vector<std::pair<size_t, size_t> > result = carve::triangulate::incorporateHolesIntoPolygon(projected_poly);
							faceLoops.push_back(std::vector<size_t>());
							std::vector<size_t> &out = faceLoops.back();
							out.reserve(result.size());
							for (size_t j = 0; j < result.size(); ++j)
							{
								if (result[j].first == 0)
									out.push_back(faceLoop[result[j].second]);
								else
									out.push_back(holes[result[j].first - 1][result[j].second]);
							}
						}
					}
				}
				else //it is a curved surface or we want a triangulation so we need to use the triangulation of the surface
				{
					const Poly_Array1OfTriangle& triangles = mesh->Triangles();
					Standard_Integer nbTriangles = mesh->NbTriangles();
					bool faceReversed = face->IsReversed;
					Standard_Integer t[3];
					for (Standard_Integer i = 1; i <= nbTriangles; i++) //add each triangle as a face
					{
						if (faceReversed) //get normals in the correct order of triangulation
							triangles(i).Get(t[2], t[1], t[0]);
						else
							triangles(i).Get(t[0], t[1], t[2]);
						faceLoops.push_back(std::vector<size_t>());
						std::vector<size_t> &faceLoop = faceLoops.back(); //get a new face

						for (size_t j = 0; j < 3; j++)
						{
							gp_XYZ p = nodes.Value(t[j]).XYZ();
							loc.Transformation().Transforms(p);
							size_t index;
							XbimPoint3DWithTolerance^ pt = gcnew XbimPoint3DWithTolerance(p.X(), p.Y(), p.Z(), tolerance);
							if (!pointMap->TryGetValue(pt, index))
							{
								index = vertices.size();
								pointMap->Add(pt, index);
								vertices.push_back(carve::geom::VECTOR(pt->X, pt->Y, pt->Z));
							}
							if (std::find(faceLoop.begin(), faceLoop.end(), index) == faceLoop.end())
								faceLoop.push_back(index);
						}
						if (faceLoop.size() < 3) faceLoops.pop_back(); //remove loop if it is a line
					}
				}

			}
			//convert the faces and vertices to a mesh set
			std::vector<face_t *> faceList;
			faceList.reserve(faceLoops.size());
			for (size_t i = 0; i < faceLoops.size(); ++i)
			{
				std::vector<vertex_t *> vf;
				size_t loopSize = faceLoops[i].size();
				vf.reserve(loopSize);
				for (size_t j = 0; j < loopSize; ++j)
				{
					vf.push_back(&vertices[faceLoops[i][j]]);
				}
				faceList.push_back(new face_t(vf.begin(), vf.end()));
			}
			std::vector<mesh_t*> meshes;
			mesh_t::create(faceList.begin(), faceList.end(), meshes, carve::mesh::MeshOptions().avoid_cavities(true), tolerance*tolerance, false);
			pMeshSet = IntPtr(new meshset_t(vertices, meshes));
		}
#pragma endregion


#pragma region Equality Overrides

		bool XbimFacetedSolid::Equals(Object^ obj)
		{
			XbimFacetedSolid^ s = dynamic_cast<XbimFacetedSolid^>(obj);
			// Check for null
			if (s == nullptr) return false;
			return this == s;
		}

		bool XbimFacetedSolid::Equals(IXbimSolid^ obj)
		{
			XbimFacetedSolid^ s = dynamic_cast<XbimFacetedSolid^>(obj);
			if (s == nullptr) return false;
			return this == s;
		}

		int XbimFacetedSolid::GetHashCode()
		{
			if (!IsValid) return 0;
			int hash = (int)IntPtr(pMeshSet);
			return hash;
		}

		bool XbimFacetedSolid::operator ==(XbimFacetedSolid^ left, XbimFacetedSolid^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			return  (const meshset_t*)left == (const meshset_t*)right;

		}

		bool XbimFacetedSolid::operator !=(XbimFacetedSolid^ left, XbimFacetedSolid^ right)
		{
			return !(left == right);
		}


#pragma endregion


#pragma region IXbimSolid Interface

		IXbimShellSet^ XbimFacetedSolid::Shells::get()
		{
			if (!IsValid) return XbimShellSet::Empty;
			meshset_t* pMSet = (meshset_t*)this;
			List<IXbimShell^>^ shells = gcnew List<IXbimShell^>(pMSet->meshes.size());
			for (size_t i = 0; i < pMSet->meshes.size(); i++)
				shells->Add(gcnew XbimFacetedShell(this, pMSet->meshes[i]));
			return gcnew XbimShellSet(shells);
		}

		IXbimFaceSet^ XbimFacetedSolid::Faces::get()
		{
			if (!IsValid) return XbimFaceSet::Empty;
			meshset_t* pMSet = (meshset_t*)this;
			List<IXbimFace^>^ faces = gcnew List<IXbimFace^>();
			for (face_iter fi = pMSet->faceBegin(); fi != pMSet->faceEnd(); fi++)
				faces->Add(gcnew XbimPolygonalFace(this, *fi));
			return gcnew XbimFaceSet(faces);
		}

		IXbimEdgeSet^ XbimFacetedSolid::Edges::get()
		{
			if (!IsValid) return XbimEdgeSet::Empty;
			List<IXbimEdge^>^ edges = gcnew List<IXbimEdge^>();
			meshset_t* pMSet = (meshset_t*)this;
			for (size_t i = 0; i < pMSet->meshes.size(); i++)
			{
				//closed edges
				for (std::vector<edge_t*>::const_iterator ei = pMSet->meshes[i]->closed_edges.begin(); ei != pMSet->meshes[i]->closed_edges.end(); ei++)
				{
					edge_t* e = *ei;
					if (e->rev != nullptr && e->rev->face == e->face) continue;//its a hole connecting edge
					XbimPoint3DWithTolerance^ start = gcnew XbimPoint3DWithTolerance(e->v1()->v.x, e->v1()->v.y, e->v1()->v.z, Tolerance);
					XbimPoint3DWithTolerance^ end = gcnew XbimPoint3DWithTolerance(e->v2()->v.x, e->v2()->v.y, e->v2()->v.z, Tolerance);

					edges->Add(gcnew XbimLinearEdge(start, end));
				}
				//now do any open edges
				for (std::vector<edge_t*>::const_iterator ei = pMSet->meshes[i]->open_edges.begin(); ei != pMSet->meshes[i]->open_edges.end(); ei++)
				{
					edge_t* e = *ei;
					if (e->rev != nullptr && e->rev->face == e->face) continue;//its a hole connecting edge
					XbimPoint3DWithTolerance^ start = gcnew XbimPoint3DWithTolerance(e->v1()->v.x, e->v1()->v.y, e->v1()->v.z, Tolerance);
					XbimPoint3DWithTolerance^ end = gcnew XbimPoint3DWithTolerance(e->v2()->v.x, e->v2()->v.y, e->v2()->v.z, Tolerance);
					edges->Add(gcnew XbimLinearEdge(start, end));
				}
			}

			return gcnew XbimEdgeSet(edges);
		}

		IXbimVertexSet^ XbimFacetedSolid::Vertices::get()
		{
			if (!IsValid) return XbimVertexSet::Empty;
			List<IXbimVertex^>^ verts = gcnew List<IXbimVertex^>();
			meshset_t* pMSet = (meshset_t*)this;
			for (size_t i = 0; i < pMSet->vertex_storage.size(); i++)
			{
				vertex_t v = pMSet->vertex_storage[i];
				verts->Add(gcnew XbimPoint3DWithTolerance(v.v.x, v.v.y, v.v.z, tolerance));
			}

			return gcnew XbimVertexSet(verts);
		}

		bool XbimFacetedSolid::IsPolyhedron::get()
		{
			if (!IsValid) return false;
			return true; //it always is
		}


		XbimRect3D XbimFacetedSolid::BoundingBox::get()
		{
			if (!IsValid) return XbimRect3D::Empty;
			meshset_t* pMSet = (meshset_t*)this;
			aabb_t aabb = pMSet->getAABB();
			if (aabb.isEmpty())return XbimRect3D::Empty;
			carve::geom::aabb<3>::vector_t min = aabb.min();
			carve::geom::aabb<3>::vector_t max = aabb.max();
			return XbimRect3D(min.x, min.y, min.z, max.x - min.x, max.y - min.y, max.z - min.z);
		}

		double XbimFacetedSolid::Volume::get()
		{
			if (!IsValid) return 0.;
			double vol = 0;
			meshset_t* pMSet = (meshset_t*)this;
			for (size_t i = 0; i < pMSet->meshes.size(); i++)
			{
				vol = vol + pMSet->meshes[i]->volume();
			}
			return vol;
		}

		double XbimFacetedSolid::SurfaceArea::get()
		{
			if (!IsValid) return 0;
			double area = 0;
			for each (IXbimFace^  face in Faces)
			{
				area += face->Area;
			}
			return area;
		}


		//returns true if the shell is a closed manifold solid
		bool XbimFacetedSolid::IsClosed::get()
		{
			if (!IsValid) return false;
			meshset_t* pMSet = (meshset_t*)this;
			return pMSet->isClosed();
		}

		IXbimGeometryObject^ XbimFacetedSolid::Transform(XbimMatrix3D t)
		{
			if (!IsValid) return this;
			carve::math::Matrix m(t.M11, t.M21, t.M31, t.OffsetX,
				t.M12, t.M22, t.M32, t.OffsetY,
				t.M13, t.M23, t.M33, t.OffsetZ,
				t.M14, t.M24, t.M34, t.M44);
			carve::math::matrix_transformation mt(m);
			meshset_t* pMSet = (meshset_t*)this;
			meshset_t* newMesh = pMSet->clone();
			newMesh->transform(mt);
			return gcnew XbimFacetedSolid(newMesh);
		}

		IXbimSolidSet^ XbimFacetedSolid::Cut(IXbimSolidSet^ toCut, double tolerance)
		{
			if (toCut->Count == 0) return gcnew XbimSolidSet(this);
			if (toCut->Count == 1) return this->Cut(toCut->First, tolerance);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Cut(toCut, tolerance);
		}

		IXbimSolidSet^ XbimFacetedSolid::Cut(IXbimSolid^ toCut, double tolerance)
		{
			if (!IsValid || !toCut->IsValid) return XbimSolidSet::Empty;
			XbimFacetedSolid^ cuttingObject = dynamic_cast<XbimFacetedSolid^>(toCut);

			if (cuttingObject == nullptr)
			{
				XbimSolid^ solidCut = dynamic_cast<XbimSolid^>(toCut);
				if (solidCut != nullptr)
				{
					if (solidCut->IsPolyhedron) //downgrade toCut to facetation, faster
					{
						cuttingObject = gcnew XbimFacetedSolid(solidCut, tolerance);
						//get on and do it
					}
					else //upgrade tocut to occ, more accurate with curves
					{
						IXbimSolid^ thisAsSolid = this->ConvertToXbimSolid();
						if (thisAsSolid != nullptr)
						{
							return thisAsSolid->Cut(solidCut, tolerance);
						}
						else
						{
							XbimGeometryCreator::logger->WarnFormat("WS011: Invalid operation. Only solid shapes can be cut from another solid");
							return gcnew XbimSolidSet(this); // the result would be no change so return this	
						}
					}
				}
				else
				{ //is it a compound of solids
					IXbimSolidSet^ solidSet = dynamic_cast<IXbimSolidSet^>(toCut);
					if (solidSet != nullptr)
					{
						XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
						return thisSolidSet->Cut(solidSet, tolerance);
					}
					else
					{
						XbimGeometryCreator::logger->WarnFormat("WF001: Invalid operation. Only solid shapes can be cut from another solid");
						return gcnew XbimSolidSet(this); // the result would be no change so return this
					}
				}
			}
			String^ err = "";
			try
			{
				carve::csg::CSG csg(tolerance);
				meshset_t* cut = csg.compute(this, cuttingObject, carve::csg::CSG::A_MINUS_B, NULL, carve::csg::CSG::CLASSIFY_NORMAL);
				GC::KeepAlive(this);
				GC::KeepAlive(cuttingObject);
				if (cut != nullptr)
				{
					size_t tally = cut->meshes.size();
					if (tally == 1) return gcnew XbimSolidSet(gcnew XbimFacetedSolid(cut));
					XbimSolidSet^ result = gcnew XbimSolidSet();

					for (int i = tally - 1; i >= 0; i--)
					{
						mesh_t* m = cut->meshes.back();
						cut->meshes.pop_back();
						m->meshset = nullptr;
						if (m->isClosed()) //it is a manifold solid
						{
							std::vector<mesh_t *> mesh;
							mesh.push_back(m);
							meshset_t* mSet = new meshset_t(mesh);
							result->Add(gcnew XbimFacetedSolid(mSet));
						}
					    else
							delete m; //throw away non manifolds
					}
					delete cut;
					return result;
				}
			}
			catch (carve::exception ce)
			{
				err = gcnew String(ce.str().c_str());
			}
			catch (System::Runtime::InteropServices::SEHException^ ex) //these should never happen, raise an error
			{
				err = ex->Message;
			}
			catch (...)
			{
				err = "Unexpected Error";
			}
			XbimGeometryCreator::logger->WarnFormat("WF002: Boolean Union operation failed. " + err);
			return XbimSolidSet::Empty;
		}

		IXbimSolidSet^ XbimFacetedSolid::Intersection(IXbimSolidSet^ toIntersect, double tolerance)
		{
			if (toIntersect->Count == 0) return gcnew XbimSolidSet(this);
			if (toIntersect->Count == 1) return this->Intersection(toIntersect->First, tolerance);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Intersection(toIntersect, tolerance);
		}

		IXbimSolidSet^ XbimFacetedSolid::Intersection(IXbimSolid^ toIntersect, double tolerance)
		{
			if (!IsValid || !toIntersect->IsValid) return XbimSolidSet::Empty;
			XbimFacetedSolid^ intersectObject = dynamic_cast<XbimFacetedSolid^>(toIntersect);

			if (intersectObject == nullptr)
			{
				XbimSolid^ solidIntersect = dynamic_cast<XbimSolid^>(toIntersect);
				if (solidIntersect != nullptr)
				{
					if (solidIntersect->IsPolyhedron) //downgrade toCut to facetation, faster
					{
						intersectObject = gcnew XbimFacetedSolid(solidIntersect, tolerance);
						//get on and do it
					}
					else //upgrade tocut to occ, more accurate with curves
					{
						IXbimSolid^ thisAsSolid = this->ConvertToXbimSolid();
						if (thisAsSolid != nullptr)
						{
							return thisAsSolid->Cut(solidIntersect, tolerance);
						}
						else
						{
							XbimGeometryCreator::logger->WarnFormat("WF005: Invalid operation. Only solid shapes can be intersect with another solid");
							return gcnew XbimSolidSet(this); // the result would be no change so return this	
						}
					}
				}
				else
				{
					XbimGeometryCreator::logger->WarnFormat("WF006: Invalid operation. Only solid shapes can be intersect with another solid");
					return gcnew XbimSolidSet(this); // the result would be no change so return this
				}
			}
			String^ err = "";
			try
			{
				carve::csg::CSG csg(tolerance);
				meshset_t* intersected = csg.compute(this, intersectObject, carve::csg::CSG::INTERSECTION, NULL, carve::csg::CSG::CLASSIFY_NORMAL);
				GC::KeepAlive(this);
				GC::KeepAlive(intersectObject);
				if (intersected != nullptr)
				{
					size_t tally = intersected->meshes.size();
					if (tally == 1) return gcnew XbimSolidSet(gcnew XbimFacetedSolid(intersected));
					XbimSolidSet^ result = gcnew XbimSolidSet();

					for (int i = tally - 1; i >= 0; i--)
					{
						mesh_t* m = intersected->meshes.back();
						intersected->meshes.pop_back();
						m->meshset = nullptr;
						if (m->isClosed()) //it is a manifold solid
						{
							std::vector<mesh_t *> mesh;
							mesh.push_back(m);
							meshset_t* mSet = new meshset_t(mesh);
							result->Add(gcnew XbimFacetedSolid(mSet));
						}
						else
							delete m; //throw away non manifolds
					}
					delete intersected;
					return result;
				}
			}
			catch (carve::exception ce)
			{
				err = gcnew String(ce.str().c_str());
			}
			catch (System::Runtime::InteropServices::SEHException^ ex) //these should never happen, raise an error
			{
				err = ex->Message;
			}
			catch (...)
			{
				err = "Unexpected Error";
			}
			XbimGeometryCreator::logger->WarnFormat("WF003: Boolean Intersect operation failed. " + err);
			return XbimSolidSet::Empty;
		}

		IXbimSolidSet^ XbimFacetedSolid::Union(IXbimSolidSet^ toUnion, double tolerance)
		{
			if (toUnion->Count == 0) return gcnew XbimSolidSet(this);
			if (toUnion->Count == 1) return this->Union(toUnion->First, tolerance);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Union(toUnion, tolerance);
		}

		IXbimSolidSet^ XbimFacetedSolid::Union(IXbimSolid^ toUnion, double tolerance)
		{
			if (!IsValid || !toUnion->IsValid) return XbimSolidSet::Empty;
			XbimFacetedSolid^ unionObject = dynamic_cast<XbimFacetedSolid^>(toUnion);

			if (unionObject == nullptr)
			{
				XbimSolid^ solidUnion = dynamic_cast<XbimSolid^>(toUnion);
				if (solidUnion != nullptr)
				{
					if (solidUnion->IsPolyhedron) //downgrade toCut to facetation, faster
					{
						unionObject = gcnew XbimFacetedSolid(solidUnion, tolerance);
						//get on and do it
					}
					else //upgrade tocut to occ, more accurate with curves
					{
						IXbimSolid^ thisAsSolid = this->ConvertToXbimSolid();
						if (thisAsSolid != nullptr)
						{
							return thisAsSolid->Cut(solidUnion, tolerance);
						}
						else
						{
							XbimGeometryCreator::logger->WarnFormat("WF007: Invalid operation. Only solid shapes can be unioned with another solid");
							return gcnew XbimSolidSet(this); // the result would be no change so return this	
						}
					}
				}
				else
				{
					XbimGeometryCreator::logger->WarnFormat("WF008: Invalid operation. Only solid shapes can be unioned with another solid");
					return gcnew XbimSolidSet(this); // the result would be no change so return this
				}
			}
			String^ err = "";
			try
			{
				carve::csg::CSG csg(tolerance);
				meshset_t* united = csg.compute(this, unionObject, carve::csg::CSG::UNION, NULL, carve::csg::CSG::CLASSIFY_NORMAL);
				GC::KeepAlive(this);
				GC::KeepAlive(unionObject);
				GC::KeepAlive(toUnion);
				if (united != nullptr)
				{
					size_t tally = united->meshes.size();
					if (tally == 1) return gcnew XbimSolidSet(gcnew XbimFacetedSolid(united));
					XbimSolidSet^ result = gcnew XbimSolidSet();

					for (int i = tally - 1; i >= 0; i--)
					{
						mesh_t* m = united->meshes.back();
						united->meshes.pop_back();
						m->meshset = nullptr;
						if (m->isClosed()) //it is a manifold solid
						{
							std::vector<mesh_t *> mesh;
							mesh.push_back(m);
							meshset_t* mSet = new meshset_t(mesh);
							result->Add(gcnew XbimFacetedSolid(mSet));
						}
						else
							delete m; //throw away non manifolds
					}
					delete united;
					return result;
				}
			}
			catch (carve::exception ce)
			{
				err = gcnew String(ce.str().c_str());
			}
			catch (System::Runtime::InteropServices::SEHException^ ex) //these should never happen, raise an error
			{
				err = ex->Message;
			}
			catch (...)
			{
				err = "Unexpected Error";
			}
			XbimGeometryCreator::logger->WarnFormat("WF002: Boolean Cut operation failed. " + err);
			return XbimSolidSet::Empty;
		}

		IXbimFaceSet^ XbimFacetedSolid::Section(IXbimFace^ toSection, double tolerance)
		{
			if (!IsValid || !toSection->IsValid) return XbimFaceSet::Empty;
			XbimFace^ faceSection = dynamic_cast<XbimFace^>(toSection);
			if (faceSection == nullptr)  throw gcnew ArgumentException("Only IXbimFaces created by Xbim.OCC modules are supported", "toSection");

			XbimFacetedSolid^ sectionFace = MakeInfiniteFace(faceSection->Location, faceSection->Normal);
			String^ err = "";
			try
			{
				carve::csg::CSG csg(tolerance);
				meshset_t* intersected = csg.compute(this, sectionFace, carve::csg::CSG::INTERSECTION, NULL, carve::csg::CSG::CLASSIFY_NORMAL);
				GC::KeepAlive(this);
				GC::KeepAlive(sectionFace);
				if (intersected != nullptr)
				{
					XbimFacetedSolid^ result = gcnew XbimFacetedSolid(intersected);
					return result->Faces;
				}
			}
			catch (carve::exception ce)
			{
				err = gcnew String(ce.str().c_str());
			}
			catch (System::Runtime::InteropServices::SEHException^ ex) //these should never happen, raise an error
			{
				err = ex->Message;
			}
			catch (...)
			{
				err = "Unexpected Error";
			}
			XbimGeometryCreator::logger->WarnFormat("WF003: Boolean Intersect operation failed. " + err);
			return XbimFaceSet::Empty;
		}

		XbimFacetedSolid^ XbimFacetedSolid::MakeInfiniteFace(XbimPoint3D l, XbimVector3D n)
		{
			gp_Pln halfSpacePlane(gp_Pnt(l.X, l.Y, l.Z), gp_Dir(n.X, n.Y, n.Z));
			Handle(Geom_Plane) pl = new Geom_Plane(halfSpacePlane);

			Standard_Real U1, U2, V1, V2;
			pl->Bounds(U1, U2, V1, V2);
			U1 /= 1e90; U2 /= 1e90; V1 /= 1e90; V2 /= 1e90;
			gp_Pnt tl, bl, tr, br; //the four points of the bounding plane
			pl->D0(U1, V1, bl);
			pl->D0(U1, V2, tl);
			pl->D0(U2, V1, br);
			pl->D0(U2, V2, tr);

			carve::input::PolyhedronData data;
			data.addVertex(carve::geom::VECTOR(tl.X(), tl.Y(), tl.Z()));
			data.addVertex(carve::geom::VECTOR(bl.X(), bl.Y(), bl.Z()));
			data.addVertex(carve::geom::VECTOR(br.X(), br.Y(), br.Z()));
			data.addVertex(carve::geom::VECTOR(tr.X(), tr.Y(), tr.Z()));
			data.addFace(0, 1, 2, 3);
			meshset_t* m = data.createMesh(carve::input::Options());
			return  gcnew XbimFacetedSolid(m);
		}
#pragma endregion

#pragma region Methods
		IXbimSolid^ XbimFacetedSolid::ConvertToXbimSolid()
		{
			if (!IsValid) return nullptr;
			//create a map to look up all the vertices
			/*std::unordered_map<vertex_t*, IXbimVertex^> lookup;

			for (size_t i = 0; i < pMeshSet->vertex_storage.size(); i++)
			{
			vertex_t* v = &(pMeshSet->vertex_storage[i]);
			lookup[v]=gcnew XbimVertex(v, tolerance);
			}*/
			BRepBuilderAPI_Sewing seamstress(tolerance);
			for each (IXbimFace^ face in Faces)
			{
				seamstress.Add(gcnew XbimFace(face));
			}
			seamstress.Perform();
			TopoDS_Shape result = seamstress.SewedShape();
			if (!result.IsNull() && result.ShapeType() == TopAbs_SHELL)
			{
				XbimShell^ shell = gcnew XbimShell(TopoDS::Shell(result));
				if (shell->IsClosed)
					return shell->MakeSolid();
			}
			XbimGeometryCreator::logger->Warn("Failed to convert faceted solid to a BRep solid");
			return nullptr;
		}

		void XbimFacetedSolid::WriteTriangulation(TextWriter^ tw, double tolerance, double deflection, double angle)
		{
			if (!IsValid) return;
			meshset_t* pMSet = (meshset_t*)this;
			int vCount = pMSet->vertex_storage.size();
			int fCount = 0, tCount = 0, nCount = 0;
			for (meshset_t::face_iter i = pMSet->faceBegin(), e = pMSet->faceEnd(); i != e; ++i) fCount++;

			Dictionary<XbimPoint3DWithTolerance^, size_t>^ normalMap = gcnew Dictionary<XbimPoint3DWithTolerance^, size_t>();
			List<XbimVector3D>^ normals = gcnew List <XbimVector3D>(fCount);
			List<size_t>^ normalIndices = gcnew List <size_t>(fCount);
			std::vector<std::vector<carve::triangulate::tri_idx>> triangulation;
			for (meshset_t::face_iter i = pMSet->faceBegin(), e = pMSet->faceEnd(); i != e; ++i)
			{

				face_t *face = *i;
				vector_t n = face->plane.N.normalized();
				if (face->nVertices() < 3 || Double::IsNaN(n.x)) continue;//skip invalid faces	
				XbimPoint3DWithTolerance^ p3d = gcnew XbimPoint3DWithTolerance(Math::Round(n.x, 4), Math::Round(n.y, 4), Math::Round(n.z, 4), tolerance);
				size_t index;
				if (!normalMap->TryGetValue(p3d, index))
				{
					index = normals->Count;
					normalMap->Add(p3d, index);
					normals->Add(XbimVector3D(p3d->X, p3d->Y, p3d->Z));
				}
				normalIndices->Add(index);
				std::vector<carve::mesh::MeshSet<3>::vertex_t *> verts;
				face->getVertices(verts);
				triangulation.push_back(std::vector<carve::triangulate::tri_idx>());
				std::vector<carve::triangulate::tri_idx>&  indices = triangulation.back();
				if (verts.size() > 3)//we need to triangulate
				{

					std::vector<carve::triangulate::tri_idx>  result;
					std::vector<carve::geom::vector<2> > projectedVerts;
					face->getProjectedVertices(projectedVerts);
					carve::triangulate::triangulate(projectedVerts, result, tolerance);
					for (size_t i = 0; i < result.size(); i++)
					{
						size_t a = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[result[i].a]);
						size_t b = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[result[i].b]);
						size_t c = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[result[i].c]);
						carve::triangulate::tri_idx triangle(a, b, c);
						indices.push_back(triangle);
						tCount++;
					}

				}
				else //just add a triangle
				{
					size_t a = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[0]);
					size_t b = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[1]);
					size_t c = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[2]);
					carve::triangulate::tri_idx triangle(a, b, c);
					indices.push_back(triangle);
					tCount++;
				}
			}
			nCount = normals->Count;
			// Write out header
			tw->WriteLine(String::Format("P {0} {1} {2} {3} {4}", 1, vCount, fCount, tCount, nCount));
			//write out vertices and normals  
			tw->Write("V");
			for (std::vector<meshset_t::vertex_t>::const_iterator i = pMSet->vertex_storage.begin(); i != pMSet->vertex_storage.end(); ++i)
			{
				vertex_t vt = *i;
				tw->Write(String::Format(" {0},{1},{2}", vt.v.x, vt.v.y, vt.v.z));
			}
			tw->WriteLine();
			tw->Write("N");
			for each (XbimVector3D n in normals) tw->Write(String::Format(" {0},{1},{2}", n.X, n.Y, n.Z));
			tw->WriteLine();
			//write out the triangulated faces
			int faceIndex = 0;
			for (std::vector<std::vector<carve::triangulate::tri_idx>>::const_iterator trianglesIt = triangulation.begin();
				trianglesIt != triangulation.end(); ++trianglesIt)
			{
				size_t normalIndex = normalIndices[faceIndex];
				std::vector<carve::triangulate::tri_idx> triangles = *trianglesIt;
				tw->Write("T");
				bool first = true;
				for (std::vector<carve::triangulate::tri_idx>::const_iterator triangleIt = triangles.begin();
					triangleIt != triangles.end(); ++triangleIt)
				{
					carve::triangulate::tri_idx triangle = *triangleIt;
					if (first)
						tw->Write(String::Format(" {0}/{3},{1},{2}", triangle.a, triangle.b, triangle.c, normalIndex));
					else
						tw->Write(String::Format(" {0},{1},{2}", triangle.a, triangle.b, triangle.c));
					first = false;
				}
				tw->WriteLine();
				faceIndex++;
			}
		}

		void XbimFacetedSolid::WriteTriangulation(BinaryWriter^ binaryWriter, double tolerance, double deflection, double angle)
		{
			if (!IsValid) return;
			meshset_t* pMSet = (meshset_t*)this;
			int vCount = pMSet->vertex_storage.size();
			int fCount = 0, tCount = 0;

			// Write out header
			binaryWriter->Write((unsigned char)1); //stream format version
			binaryWriter->Write((UInt32)vCount); //number of vertices
			long long tPos = binaryWriter->Seek(0, SeekOrigin::Current);
			binaryWriter->Write((UInt32)tCount); //number of triangles
			//write out vertices 
			for (std::vector<meshset_t::vertex_t>::const_iterator i = pMSet->vertex_storage.begin(); i != pMSet->vertex_storage.end(); ++i)
			{
				vertex_t vt = *i;
				binaryWriter->Write((float)vt.v.x);
				binaryWriter->Write((float)vt.v.y);
				binaryWriter->Write((float)vt.v.z);

			}
			long long fPos = binaryWriter->Seek(0, SeekOrigin::Current);
			binaryWriter->Write((Int32)fCount);
			for (meshset_t::face_iter i = pMSet->faceBegin(), e = pMSet->faceEnd(); i != e; ++i)
			{
				face_t *face = *i;
				vector_t n = face->plane.N.normalized();
				if (face->nVertices() < 3 || Double::IsNaN(n.x)) continue;//skip invalid faces	
				XbimPackedNormal packedNormal(n.x, n.y, n.z);
				
				fCount++;
				std::vector<carve::mesh::MeshSet<3>::vertex_t *> verts;
				face->getVertices(verts);
				std::vector<carve::triangulate::tri_idx> indices;		
				if (verts.size() > 3)//we need to triangulate
				{
					std::vector<carve::triangulate::tri_idx>  result;
					std::vector<carve::geom::vector<2> > projectedVerts;
					face->getProjectedVertices(projectedVerts);
					carve::triangulate::triangulate(projectedVerts, result, tolerance);
					for (size_t i = 0; i < result.size(); i++)
					{
						size_t a = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[result[i].a]);
						size_t b = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[result[i].b]);
						size_t c = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[result[i].c]);
						carve::triangulate::tri_idx triangle(a, b, c);
						indices.push_back(triangle);
						tCount++;
					}
				}
				else //just add a triangle
				{
					size_t a = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[0]);
					size_t b = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[1]);
					size_t c = (size_t)carve::poly::ptrToIndex_fast(pMSet->vertex_storage, verts[2]);
					carve::triangulate::tri_idx triangle(a, b, c);
					indices.push_back(triangle);
					tCount++;
				}
				//write the numner of triangles
				binaryWriter->Write((Int32)indices.size()); //positive to indicate planar face
				//write the normal
				packedNormal.Write(binaryWriter); //write the normal for the face
				//write out the indices
				for (std::vector<carve::triangulate::tri_idx>::const_iterator triangleIt = indices.begin();
					triangleIt != indices.end(); ++triangleIt)
				{
					carve::triangulate::tri_idx triangle = *triangleIt;
					XbimOccShape::WriteIndex(binaryWriter, triangle.a, vCount);
					XbimOccShape::WriteIndex(binaryWriter, triangle.b, vCount);
					XbimOccShape::WriteIndex(binaryWriter, triangle.c, vCount);
				}

			}
			//set the totals
			long long cPos = binaryWriter->Seek(0, SeekOrigin::Current); //where are we now
			binaryWriter->Seek((int)tPos, SeekOrigin::Begin);
			binaryWriter->Write((UInt32)tCount); //number of triangles
			binaryWriter->Seek((int)fPos, SeekOrigin::Begin);
			binaryWriter->Write((Int32)fCount);
			binaryWriter->Seek((int)cPos, SeekOrigin::Current); //reset position
		}


		int XbimFacetedSolid::MergeCoPlanarFaces(double normalAngle)
		{
			if (!IsValid) return 0;
			carve::mesh::MeshSimplifier simplifier;
			meshset_t* pMSet = (meshset_t*)this;
			size_t mods = simplifier.mergeCoplanarFaces(pMSet, normalAngle);
			GC::KeepAlive(this);
			return (int)mods;
		}

		//merges every faceted solid into a single solid, if solids overlap they are unioned
		//the result is a compound of solids, it is not exposed becuase it breaks the model, it is only used for 
		//computational performance of booleans of large collections of faceted solids
		XbimFacetedSolid^ XbimFacetedSolid::Merge(IXbimSolidSet^ facetedSolids, double tolerance)
		{

			//first remove any that intersect as simple merging leads to illegal geometries.
			Dictionary<XbimFacetedSolid^, HashSet<XbimFacetedSolid^>^>^ clusters = gcnew Dictionary<XbimFacetedSolid^, HashSet<XbimFacetedSolid^>^>();
			for each (IXbimSolid^ solidToCheck in facetedSolids) //init all the clusters
			{
				XbimFacetedSolid^ polyToCheck = dynamic_cast<XbimFacetedSolid^>(solidToCheck);
				if (polyToCheck != nullptr)
					clusters[polyToCheck] = gcnew HashSet<XbimFacetedSolid^>();
			}
			if (clusters->Count == 0)
				return nullptr; //nothing to do
			if (clusters->Count == 1) //just one so return it
			{
				for each(XbimFacetedSolid^ fsolid in clusters->Keys) //take the first one
					return fsolid;
			}
			for each (XbimFacetedSolid^ solidToCheck in facetedSolids)
			{
				XbimFacetedSolid^ polyToCheck = dynamic_cast<XbimFacetedSolid^>(solidToCheck);
				if (polyToCheck != nullptr)
				{
					XbimRect3D bbToCheck = polyToCheck->BoundingBox;
					for each (KeyValuePair<XbimFacetedSolid^, HashSet<XbimFacetedSolid^>^>^ cluster in clusters)
					{
						if (polyToCheck != cluster->Key && bbToCheck.Intersects(cluster->Key->BoundingBox))
							cluster->Value->Add(polyToCheck);
					}
				}
			}
			List<XbimFacetedSolid^>^ toMergeReduced = gcnew List<XbimFacetedSolid^>();
			Dictionary<XbimFacetedSolid^, HashSet<XbimFacetedSolid^>^>^ clustersSparse = gcnew Dictionary<XbimFacetedSolid^, HashSet<XbimFacetedSolid^>^>();
			for each (KeyValuePair<XbimFacetedSolid^, HashSet<XbimFacetedSolid^>^>^ cluster in clusters)
			{
				if (cluster->Value->Count > 0)
					clustersSparse->Add(cluster->Key, cluster->Value);
				else
					toMergeReduced->Add(cluster->Key); //record the ones to simply merge
			}
			clusters = nullptr;

			XbimFacetedSolid^ clusterAround = nullptr;
			for each(XbimFacetedSolid^ fsolid in clustersSparse->Keys) //take the first one
			{
				clusterAround = fsolid;
				break;
			}

			while (clusterAround != nullptr)
			{
				HashSet<XbimFacetedSolid^>^ connected = gcnew HashSet<XbimFacetedSolid^>();
				XbimFacetedSolid::GetConnected(connected, clustersSparse, clusterAround);
				XbimFacetedSolid^ poly = nullptr;
				for each (XbimFacetedSolid^ toConnect in connected) //join up the connected
				{
					if (poly == nullptr) 
						poly = toConnect;
					else
					{
						carve::csg::CSG csg(tolerance);
						meshset_t* united = csg.compute(poly, toConnect, carve::csg::CSG::UNION, NULL, carve::csg::CSG::CLASSIFY_NORMAL);
						if (united != nullptr)
						{
							poly = gcnew XbimFacetedSolid(united);
						}
					}
				}
				if (poly != nullptr) toMergeReduced->Add(poly);
				
				for each (XbimFacetedSolid^ p in connected) //remove what we have conected
					clustersSparse->Remove(p);

				clusterAround = nullptr;
				for each(XbimFacetedSolid^ fsolid in clustersSparse->Keys) //take the first one
				{
					clusterAround = fsolid;
					break;
				}

			}

			//create a map between old and new vertices
			std::unordered_map<vertex_t *, size_t> vert_idx;

			size_t meshCount = 0;
			for each (XbimFacetedSolid^ poly in toMergeReduced)
			{
				meshCount += poly->Shells->Count;
				for (face_iter f = ((meshset_t*)poly)->faceBegin(); f != ((meshset_t*)poly)->faceEnd(); ++f)
				{
					face_t* face = *f;
					edge_t* e = face->edge;
					do
					{
						vert_idx[e->vert] = 0;
						e = e->next;
					} while (e != face->edge);
				}
			}
			//determine max number of new vertices
			std::vector<vertex_t> newVertexStorage;
			newVertexStorage.reserve(vert_idx.size());
			//Add indexes and unique vertices in to the new vertex storage
			for (std::unordered_map<vertex_t *, size_t>::iterator
				i = vert_idx.begin(); i != vert_idx.end(); ++i)
			{
				(*i).second = newVertexStorage.size();
				newVertexStorage.push_back(*(*i).first);
			}

			//create meshes and faces
			std::vector<mesh_t*> newMeshes;
			newMeshes.reserve(meshCount);
			std::vector<carve::mesh::MeshSet<3>::vertex_t *> faceVerts;
			std::vector<face_t *> faceList;
			for each (XbimFacetedSolid^ poly in toMergeReduced)
			{
				for (size_t i = 0; i < ((const meshset_t*)poly)->meshes.size(); i++)
				{
					mesh_t* mesh = ((const meshset_t*)poly)->meshes[i];
					faceList.clear();
					faceList.reserve(mesh->faces.size());
					for (std::vector<face_t*>::iterator f = mesh->faces.begin(); f != mesh->faces.end(); ++f)
					{
						face_t* face = *f;
						edge_t* e = face->edge;
						std::vector<carve::mesh::MeshSet<3>::vertex_t *> faceVerts;
						faceVerts.clear();
						do
						{
							faceVerts.push_back(&(newVertexStorage[vert_idx[e->vert]]));
							e = e->next;
						} while (e != face->edge);
						faceList.push_back(new face_t(faceVerts.begin(), faceVerts.end()));
					}
					std::vector<mesh_t*> nextMeshes;
					mesh_t::create(faceList.begin(), faceList.end(), nextMeshes, carve::mesh::MeshOptions(), false);
					for (size_t i = 0; i < nextMeshes.size(); i++)
						newMeshes.push_back(nextMeshes[i]);
				}
			}
			meshset_t* meshSet = new meshset_t(newVertexStorage, newMeshes);
			//create a new Poly 
			XbimFacetedSolid^ result = gcnew XbimFacetedSolid(meshSet);

			return result;
		}

		void  XbimFacetedSolid::GetConnected(HashSet<XbimFacetedSolid^>^ connected, Dictionary<XbimFacetedSolid^, HashSet<XbimFacetedSolid^>^>^ clusters, XbimFacetedSolid^ clusterAround)
		{
			if (connected->Add(clusterAround))
			{
				for each (KeyValuePair<XbimFacetedSolid^, HashSet<XbimFacetedSolid^>^>^ polysets in clusters)
				{
					if (!connected->Contains(polysets->Key) && !(polysets->Key == clusterAround) && polysets->Value->Contains(clusterAround))  //don't do the same one twice
					{
						GetConnected(connected, clusters, polysets->Key);
						for each (XbimFacetedSolid^ poly in polysets->Value)
						{
							GetConnected(connected, clusters, poly);
						}
					}
				}
			}
		}
#pragma endregion



	}
}
#endif // USE_CARVE_CSG
