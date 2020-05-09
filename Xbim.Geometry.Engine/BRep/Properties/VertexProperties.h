#pragma once
#include <TopoDS_Vertex.hxx>
#include <BRep_Tool.hxx>
namespace Xbim
{
	namespace Geometry
	{
		public ref class VertexProperties
		{
		private:
			double x;
			double y;
			double z;
			double tolerance;
			bool reversed;

		public:
			VertexProperties(const TopoDS_Vertex& v)
			{
				gp_Pnt p = BRep_Tool::Pnt(v);
				x = p.X();
				y = p.Y();
				z = p.Z();
				tolerance = BRep_Tool::Tolerance(v);	
				reversed = v.Orientation() == TopAbs_Orientation::TopAbs_REVERSED;
			}
			virtual property double X {double get() { return x; }; }
			virtual property double Y {double get() { return y; }; }
			virtual property double Z {double get() { return z; }; }
			virtual property double Tolerance {double get() { return tolerance; }; }
			virtual property bool Reversed {bool get() { return reversed; }; }
		};
	}
}

