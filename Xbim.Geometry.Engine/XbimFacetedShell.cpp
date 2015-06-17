#ifdef USE_CARVE_CSG

#include "XbimFacetedShell.h"
#include "XbimFaceSet.h"
#include "XbimEdgeSet.h"
#include "XbimVertexSet.h"
#include "XbimPoint3DWithTolerance.h"
#include "XbimPolygonalFace.h"
#include "XbimLinearEdge.h"

#include <carve\math.hpp>
#include <carve\matrix.hpp>
namespace Xbim
{
	namespace Geometry
	{

		XbimFacetedShell::XbimFacetedShell(XbimFacetedSolid^ fsolid, mesh_t* mesh)
		{
			facetedSolid = fsolid;
			pMesh = mesh;
		}


#pragma region Equality Overrides

		bool XbimFacetedShell::Equals(Object^ obj)
		{
			XbimFacetedShell^ s = dynamic_cast<XbimFacetedShell^>(obj);
			// Check for null
			if (s == nullptr) return false;
			return this == s;
		}

		bool XbimFacetedShell::Equals(IXbimShell^ obj)
		{
			XbimFacetedShell^ s = dynamic_cast<XbimFacetedShell^>(obj);
			if (s == nullptr) return false;
			return this == s;
		}

		int XbimFacetedShell::GetHashCode()
		{
			if (!IsValid) return 0;
			int hash = (int)IntPtr(pMesh);
			return hash;
		}

		bool XbimFacetedShell::operator ==(XbimFacetedShell^ left, XbimFacetedShell^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			return  (const mesh_t*)left == (const mesh_t*)right;

		}

		bool XbimFacetedShell::operator !=(XbimFacetedShell^ left, XbimFacetedShell^ right)
		{
			return !(left == right);
		}


#pragma endregion


#pragma region IXbimShell Interface


		IXbimFaceSet^ XbimFacetedShell::Faces::get()
		{
			if (!IsValid) return XbimFaceSet::Empty;
			List<IXbimFace^>^ faces = gcnew List<IXbimFace^>();
			for (std::vector<face_t*>::const_iterator fi = pMesh->faces.begin(); fi != pMesh->faces.end(); fi++)
				faces->Add(gcnew XbimPolygonalFace(facetedSolid, *fi));
			return gcnew XbimFaceSet(faces);
		}

		IXbimEdgeSet^ XbimFacetedShell::Edges::get()
		{
			if (!IsValid) return XbimEdgeSet::Empty;
			List<IXbimEdge^>^ edges = gcnew List<IXbimEdge^>();
			//closed edges
			for (std::vector<edge_t*>::const_iterator ei = pMesh->closed_edges.begin(); ei != pMesh->closed_edges.end(); ei++)
			{
				edge_t* e = *ei;
				XbimPoint3DWithTolerance^ start = gcnew XbimPoint3DWithTolerance(e->v1()->v.x, e->v1()->v.y, e->v1()->v.z, facetedSolid->Tolerance);
				XbimPoint3DWithTolerance^ end = gcnew XbimPoint3DWithTolerance(e->v2()->v.x, e->v2()->v.y, e->v2()->v.z, facetedSolid->Tolerance);
				edges->Add(gcnew XbimLinearEdge(start, end));
			}
			//now do any open edges
			for (std::vector<edge_t*>::const_iterator ei = pMesh->open_edges.begin(); ei != pMesh->open_edges.end(); ei++)
			{
				edge_t* e = *ei;
				XbimPoint3DWithTolerance^ start = gcnew XbimPoint3DWithTolerance(e->v1()->v.x, e->v1()->v.y, e->v1()->v.z, facetedSolid->Tolerance);
				XbimPoint3DWithTolerance^ end = gcnew XbimPoint3DWithTolerance(e->v2()->v.x, e->v2()->v.y, e->v2()->v.z, facetedSolid->Tolerance);
				edges->Add(gcnew XbimLinearEdge(start, end));
			}
			return gcnew XbimEdgeSet(edges);
		}

		IXbimVertexSet^ XbimFacetedShell::Vertices::get()
		{
			if (!IsValid) return XbimVertexSet::Empty;
			//if (pMesh->meshset->meshes.size() == 1) //there is just this one so return from vertex store
			//	return facetedSolid->Vertices;
			std::unordered_set<vertex_t*> vertices;
			for (std::vector<face_t*>::const_iterator fi = pMesh->faces.begin(); fi != pMesh->faces.end(); fi++)
			{
				face_t* pface = *fi;
				std::vector<vertex_t *> verts;
				pface->getVertices(verts);
				for (size_t i = 0; i < verts.size(); i++)
				{
					vertices.insert(verts[i]);
				}
			}
			List<IXbimVertex^>^ xverts = gcnew List<IXbimVertex^>();
			for (std::unordered_set<vertex_t*>::iterator it = vertices.begin(); it != vertices.end(); it++)
			{
				vertex_t* v = *it;
				xverts->Add(gcnew XbimPoint3DWithTolerance(v->v.x, v->v.y, v->v.z, facetedSolid->Tolerance));
			}
			return gcnew XbimVertexSet(xverts);

		}

		bool XbimFacetedShell::IsPolyhedron::get()
		{
			if (!IsValid) return false;
			return true; //it always is
		}

		

		XbimRect3D XbimFacetedShell::BoundingBox::get()
		{
			if (!IsValid) return XbimRect3D::Empty;
			aabb_t aabb = pMesh->getAABB();
			if (aabb.isEmpty())return XbimRect3D::Empty;
			carve::geom::aabb<3>::vector_t min = aabb.min();
			carve::geom::aabb<3>::vector_t max = aabb.max();
			return XbimRect3D(min.x, min.y, min.z, max.x - min.x, max.y - min.y, max.z - min.z);
		}

		double XbimFacetedShell::Volume::get()
		{
			if (!IsValid) return 0.;
			else return pMesh->volume();
		}

		double XbimFacetedShell::SurfaceArea::get()
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
		bool XbimFacetedShell::IsClosed::get()
		{
			if (!IsValid) return false;
			return pMesh->isClosed();
		}

		IXbimGeometryObject^ XbimFacetedShell::Transform(XbimMatrix3D t)
		{
			if (!IsValid) return this;

			const meshset_t* ms = facetedSolid;
			size_t i;
			for (i = 0; i < ms->meshes.size(); i++)
				if (ms->meshes[i] == pMesh) break;
			XbimFacetedSolid^ transformedSolid = (XbimFacetedSolid^)facetedSolid->Transform(t);
			return gcnew XbimFacetedShell(transformedSolid, ((const meshset_t*)transformedSolid)->meshes[i]);
		}


		IXbimGeometryObject^ XbimFacetedShell::Cut(IXbimGeometryObject^ toCut, double tolerance)
		{
			throw gcnew NotImplementedException("Booleans not implemented");
		}

		IXbimGeometryObject^ XbimFacetedShell::Intersection(IXbimGeometryObject^ toIntersect, double tolerance)
		{
			throw gcnew NotImplementedException("Booleans not implemented");
		}

		IXbimGeometryObject^ XbimFacetedShell::Union(IXbimGeometryObject^ toUnion, double tolerance)
		{
			throw gcnew NotImplementedException("Booleans not implemented");
		}

		IXbimFaceSet^ XbimFacetedShell::Section(IXbimFace^ toSection, double tolerance)
		{
			throw gcnew NotImplementedException("Booleans not implemented");
		}


#pragma endregion



	}
}

#endif // USE_CARVE_CSG
