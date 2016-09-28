#pragma once
#include "XbimGeometryObject.h"
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		ref struct XbimPoint3DWithTolerance:IXbimPoint, IXbimVertex
		{
		private:
			XbimPoint3D point;
			double tolerance;
			
		public:
#pragma region destructors

			~XbimPoint3DWithTolerance(){ }
			!XbimPoint3DWithTolerance(){ }

#pragma endregion
			XbimPoint3DWithTolerance(double x, double y, double z, double tolerance);
			XbimPoint3DWithTolerance(XbimPoint3D point, double t);
			XbimPoint3DWithTolerance(IXbimPoint^ point);
			XbimPoint3DWithTolerance(IIfcPointOnCurve^ point);
			XbimPoint3DWithTolerance(IIfcPointOnSurface^ point);
#pragma region Interface Overrides
			virtual property bool IsValid{bool get() { return true; }; }
			virtual property bool IsSet{bool get() { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get()  { return XbimGeometryObjectType::XbimPointType; }; }
			virtual property double X{double get(){ return point.X; }; }
			virtual property double Y{double get(){ return point.Y; }; }
			virtual property double Z{double get(){ return point.Z; }; }
			virtual property XbimPoint3D Point{XbimPoint3D get(){ return point; }; }
			virtual property double Tolerance{double get(){ return tolerance; }; }
			virtual property XbimPoint3D VertexGeometry {XbimPoint3D get() { return point; }; }
			virtual property XbimRect3D BoundingBox {XbimRect3D get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D);
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);
			virtual property String^  ToBRep{String^ get(); }
#pragma endregion

#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimPoint3DWithTolerance^ left, XbimPoint3DWithTolerance^ right);
			static bool operator !=(XbimPoint3DWithTolerance^ left, XbimPoint3DWithTolerance^ right);
			virtual bool Equals(IXbimPoint^ p);
			virtual bool Equals(IXbimVertex^ v);
#pragma endregion

			// Inherited via IXbimPoint
			virtual property Object^  Tag {Object^ get() { return nullptr; }; void set(Object^ value) { throw gcnew Exception("XbimPoint3DWithTolerance does not support Tag setting"); }; }
		};
	}
}

