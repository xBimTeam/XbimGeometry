#include "XbimPoint3DWithTolerance.h"
#include "XbimWire.h"
#include "XbimFace.h"
#include <cmath>
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		void XbimPoint3DWithTolerance::CalculateHashCode()
		{
			double gridDim = tolerance * 10.; //coursen  up
											  //This hashcode snaps points to a grid of 10 * tolerance to ensure similar points fall into the same hash cell
			double xs = point.X - std::fmod(point.X, gridDim);
			double ys = point.Y - std::fmod(point.Y, gridDim);
			double zs = point.Z - std::fmod(point.Z, gridDim);
			int hash = (int)2166136261;
			hash = hash * 16777619 ^ xs.GetHashCode();
			hash = hash * 16777619 ^ ys.GetHashCode();
			hashCode = hash * 16777619 ^ zs.GetHashCode();
		}

		XbimPoint3DWithTolerance::XbimPoint3DWithTolerance(XbimPoint3D p, double t)
		{
			point = p;
			tolerance = t; 
			CalculateHashCode();
		}
		
		XbimPoint3DWithTolerance::XbimPoint3DWithTolerance(double x, double y, double z, double t)
		{
			point = XbimPoint3D(x, y, z);
			tolerance = t; 
			CalculateHashCode();
		}
		XbimPoint3DWithTolerance::XbimPoint3DWithTolerance(IXbimPoint^ p)
		{
			point = p->Point;
			tolerance = p->Tolerance;
			CalculateHashCode();
			
		}

		XbimPoint3DWithTolerance::XbimPoint3DWithTolerance(IIfcPointOnCurve^ point, ILogger^ logger)
		{
			XbimWire^ w = gcnew XbimWire(point->BasisCurve, logger);
			this->point = w->PointAtParameter(point->PointParameter);
			this->tolerance = point->Model->ModelFactors->Precision;
			CalculateHashCode();
		}

		XbimPoint3DWithTolerance::XbimPoint3DWithTolerance(IIfcPointOnSurface^ point, ILogger^ logger)
		{
			XbimFace^ f = gcnew XbimFace(point->BasisSurface, logger);
			this->point = f->PointAtParameters(point->PointParameterU, point->PointParameterV);
			this->tolerance = point->Model->ModelFactors->Precision;
			CalculateHashCode();
		}

		String^ XbimPoint3DWithTolerance::ToBRep::get()
		{
			return String::Empty;
		}

#pragma region Equality Overrides

		bool XbimPoint3DWithTolerance::Equals(Object^ obj)
		{
			// Check for null
			if (obj == nullptr) return false;

			// Check for type
			if (this->GetType() != obj->GetType()) return false;
		
			return this == (XbimPoint3DWithTolerance^)obj;
		}

		bool XbimPoint3DWithTolerance::Equals(IXbimPoint^ obj)
		{
			// Check for null

			if (obj == nullptr) return false;
			XbimPoint3DWithTolerance^ pt = dynamic_cast<XbimPoint3DWithTolerance^>(obj);
			if (pt == nullptr) pt = gcnew XbimPoint3DWithTolerance(obj);
			return this == pt;
		}
		bool XbimPoint3DWithTolerance::Equals(IXbimVertex^ obj)
		{
			XbimPoint3DWithTolerance^ pt = dynamic_cast<XbimPoint3DWithTolerance^>(obj);
			// Check for null
			if (pt == nullptr) return false;
			return this == pt;
		}

		int XbimPoint3DWithTolerance::GetHashCode()
		{			
			return hashCode; //we can precacluate this as essentially this object is immutable
		}

		bool XbimPoint3DWithTolerance::operator ==(XbimPoint3DWithTolerance^ left, XbimPoint3DWithTolerance^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			double d = 0, dd;
			double x1 = left->X, y1 = left->Y, z1 = left->Z, x2 = right->X, y2 = right->Y, z2 = right->Z;
			dd = x1; dd -= x2; dd *= dd; d += dd;
			dd = y1; dd -= y2; dd *= dd; d += dd;
			dd = z1; dd -= z2; dd *= dd; d += dd;
			double mt = Math::Max(left->Tolerance, right->Tolerance);
			return d <= mt*mt;

		}

		bool XbimPoint3DWithTolerance::operator !=(XbimPoint3DWithTolerance^ left, XbimPoint3DWithTolerance^ right)
		{
			return !(left == right);
		}


#pragma endregion

		XbimRect3D XbimPoint3DWithTolerance::BoundingBox::get()
		{	
			return XbimRect3D(point.X, point.Y, point.Z, 0, 0, 0);
		}

		IXbimGeometryObject^ XbimPoint3DWithTolerance::TransformShallow(XbimMatrix3D matrix3D)
		{
			XbimPoint3D temp = matrix3D.Transform(point);
			return gcnew XbimPoint3DWithTolerance(temp, tolerance);
		}

		IXbimGeometryObject^ XbimPoint3DWithTolerance::Transform(XbimMatrix3D matrix3D)
		{
			XbimPoint3D temp = matrix3D.Transform(point);
			return gcnew XbimPoint3DWithTolerance(temp, tolerance);
		}
	}
}
