#include "XbimVertex.h"
#include "BRep_Tool.hxx"
#include "XbimPoint.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			double XbimVertex::Tolerance::get()
			{
				return BRep_Tool::Tolerance(OccHandle());
			}
			IXPoint^ XbimVertex::VertexGeometry::get()
			{
				
				return gcnew XbimPoint(BRep_Tool::Pnt(OccHandle()));
			}
		}
	}
}