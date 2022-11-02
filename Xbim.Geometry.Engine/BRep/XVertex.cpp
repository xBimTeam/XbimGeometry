#include "XVertex.h"
#include "BRep_Tool.hxx"
#include "XPoint.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			double XVertex::Tolerance::get()
			{
				return BRep_Tool::Tolerance(OccVertex());
			}
			IXPoint^ XVertex::VertexGeometry::get()
			{
				
				return gcnew XPoint(BRep_Tool::Pnt(OccVertex()));
			}
		}
	}
}