#include "XbimLinearEdge.h"

namespace Xbim
{
	namespace Geometry
	{
		
		XbimLinearEdge::XbimLinearEdge(XbimPoint3DWithTolerance^ start, XbimPoint3DWithTolerance^ end)
		{
			this->start = start;
			this->end = end;
		}


#pragma region Equality Overrides

		bool XbimLinearEdge::Equals(Object^ obj)
		{
			XbimLinearEdge^ e = dynamic_cast< XbimLinearEdge^>(obj);
			// Check for null
			if (e == nullptr) return false;
			return this == e;
		}

		bool XbimLinearEdge::Equals(IXbimEdge^ obj)
		{
			XbimLinearEdge^ e = dynamic_cast< XbimLinearEdge^>(obj);
			// Check for null
			if (e == nullptr) return false;
			return this == e;
		}

		int XbimLinearEdge::GetHashCode()
		{
			if (!IsValid) return 0;
			return start->GetHashCode() ^ end->GetHashCode();
		}

		bool XbimLinearEdge::operator ==(XbimLinearEdge^ left, XbimLinearEdge^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			//this edge comparer does not consider orientation
			return  left->EdgeStart == right->EdgeStart && left->EdgeEnd == right->EdgeEnd;

		}

		bool XbimLinearEdge::operator !=(XbimLinearEdge^ left, XbimLinearEdge^ right)
		{
			return !(left == right);
		}


#pragma endregion

#pragma region IXbimEdge Interface
		
		double XbimLinearEdge::Length::get()
		{
			if (IsValid)
			{
				return (start->VertexGeometry - end->VertexGeometry).Length;
			}
			else
				return 0;
		}
		XbimRect3D XbimLinearEdge::BoundingBox::get()
		{
			XbimRect3D left(start->VertexGeometry.X, start->VertexGeometry.Y, start->VertexGeometry.Z, 0, 0, 0);
			left.Union(XbimPoint3D(end->VertexGeometry));
			return left;
		}

		IXbimGeometryObject^ XbimLinearEdge::Transform(XbimMatrix3D matrix3D)
		{
			XbimPoint3DWithTolerance^ p1 = (XbimPoint3DWithTolerance^)start->Transform(matrix3D);
			XbimPoint3DWithTolerance^ p2 = (XbimPoint3DWithTolerance^)end->Transform(matrix3D);			
			return gcnew XbimLinearEdge(p1, p2);
		}
#pragma endregion

	}
}
