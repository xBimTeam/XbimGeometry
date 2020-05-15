#include "XbimWire.h"
#include "XbimEdge.h"
#include <BRepTools_WireExplorer.hxx>
using namespace System::Linq;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IEnumerable<IXEdge^>^ XbimWire::EdgeLoop::get()
			{
				BRepTools_WireExplorer wireEx(OccHandle());
				if (!wireEx.More()) return Enumerable::Empty<IXEdge^>();
				List<IXEdge^>^ edges = gcnew List<IXEdge^>();
				for (; wireEx.More(); wireEx.Next())
					edges->Add(gcnew XbimEdge(wireEx.Current()));
				return edges;
			}
		}
	}
}