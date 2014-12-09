#ifdef USE_CARVE_CSG
#include "XbimPolygonalFace.h"
#include "XbimWireSet.h"

#pragma region Carve CSG Headers

#include <carve\Triangulator.hpp>
#include <carve\Input.hpp>

#pragma endregion

namespace Xbim
{
	namespace Geometry
	{
		XbimPolygonalFace::XbimPolygonalFace(XbimFacetedSolid^ facetedSolid, face_t* face)
		{
			this->facetedSolid = facetedSolid;
			pFace = face;
		}


#pragma region IXbimFace Interface

		IXbimWire^ XbimPolygonalFace::OuterBound::get()
		{
			if (!IsValid) return nullptr;
			edge_t* edge = pFace->edge;
			std::vector<gp_Pnt> points;
			do
			{
				points.push_back(gp_Pnt(edge->vert->v.x, edge->vert->v.y, edge->vert->v.z));
				if (edge->rev != nullptr && edge->rev->face == pFace) //we are going in to holes
					edge = edge->rev->next; //avoid holes
				else
					edge = edge->next;
			} while (edge != pFace->edge);

			return gcnew XbimWire(points, facetedSolid->Tolerance);
		}

		IXbimWireSet^ XbimPolygonalFace::InnerBounds::get()
		{
			if (!IsValid) return XbimWireSet::Empty; //return an empty list, avoid using Enumberable::Empty to avoid LINQ dependencies
			edge_t* edge = pFace->edge;
			List<IXbimWire^>^ wires = gcnew List<IXbimWire^>();
			do //go around the outer loop
			{
				if (edge->rev != nullptr && edge->rev->face == pFace) //we are going in to holes
				{
					ProcessBound(edge->next, wires);
					edge = edge->rev->next; //avoid holes
				}
				else
					edge = edge->next;
			} while (edge != pFace->edge);
			return gcnew XbimWireSet(wires);
		}

		void XbimPolygonalFace::ProcessBound(edge_t* start, List<IXbimWire^>^ wires)
		{
			edge_t* edge = start;
			std::vector<gp_Pnt> points;
			do
			{
				points.push_back(gp_Pnt(edge->vert->v.x, edge->vert->v.y, edge->vert->v.z));
				if (edge->rev != nullptr && edge->rev->face == edge->face) //we are going in to holes
				{
					ProcessBound(edge->next, wires);
					edge = edge->rev->next; //avoid holes
				}
				else
					edge = edge->next;
			} while (edge->vert != start->vert);
			wires->Add(gcnew XbimWire(points, facetedSolid->Tolerance));

		}

		double XbimPolygonalFace::Area::get()
		{
			if (!IsValid) return 0;
			std::vector<carve::geom::vector<2> > projectedVerts;
			pFace->getProjectedVertices(projectedVerts);
			return Math::Abs(carve::geom2d::signedArea(projectedVerts));
		}

		double XbimPolygonalFace::Perimeter::get()
		{
			if (!IsValid) return 0;

			double perimLen = 0;
			edge_t* edge = pFace->edge;
			std::vector<gp_Pnt> points;
			do
			{
				perimLen += edge->length();
				if (edge->rev != nullptr && edge->rev->face == pFace) //we are going in to holes
					edge = edge->rev->next; //avoid holes
				else
					edge = edge->next;
			} while (edge != pFace->edge);
			return perimLen;
		}

		IXbimGeometryObject^ XbimPolygonalFace::Transform(XbimMatrix3D t)
		{
			if (!IsValid) return this;
			throw gcnew Exception("Cloning of Polygonal faces not currently implemented");
		}

		XbimRect3D XbimPolygonalFace::BoundingBox::get()
		{
			if (!IsValid) return XbimRect3D::Empty;
			aabb_t aabb = pFace->getAABB();
			if (aabb.isEmpty())return XbimRect3D::Empty;
			carve::geom::aabb<3>::vector_t min = aabb.min();
			carve::geom::aabb<3>::vector_t max = aabb.max();
			return XbimRect3D(min.x, min.y, min.z, max.x - min.x, max.y - min.y, max.z - min.z);
		}

		XbimVector3D XbimPolygonalFace::Normal::get()
		{
			if (!IsValid) return XbimVector3D();
			vector_t n = pFace->plane.N;
			return XbimVector3D(n.x, n.y, n.z);
		}

		bool XbimPolygonalFace::IsPlanar::get()
		{
			return true;
		}


#pragma endregion

#pragma region Equality Overrides

		bool XbimPolygonalFace::Equals(Object^ obj)
		{
			XbimPolygonalFace^ f = dynamic_cast<XbimPolygonalFace^>(obj);
			if (f == nullptr) return false;
			return this == f;
		}

		bool XbimPolygonalFace::Equals(IXbimFace^ obj)
		{
			XbimPolygonalFace^ f = dynamic_cast<XbimPolygonalFace^>(obj);
			if (f == nullptr) return false;
			return this == f;
		}

		int XbimPolygonalFace::GetHashCode()
		{
			if (!IsValid) return 0;
			int hash = (int)IntPtr(pFace);
			return hash;
		}

		bool XbimPolygonalFace::operator ==(XbimPolygonalFace^ left, XbimPolygonalFace^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			return  (const face_t*)left == (const face_t*)right;
		}

		bool XbimPolygonalFace::operator !=(XbimPolygonalFace^ left, XbimPolygonalFace^ right)
		{
			return !(left == right);
		}


#pragma endregion

	}
}

#endif // USE_CARVE_CSG
