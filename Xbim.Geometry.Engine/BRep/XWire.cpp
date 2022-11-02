#include "XWire.h"
#include "XEdge.h"
#include <BRepTools_WireExplorer.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <GProp_PGProps.hxx>
#include <BRepGProp.hxx>
#include <ShapeAnalysis.hxx>
using namespace System::Linq;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			
			IEnumerable<IXEdge^>^ XWire::EdgeLoop::get()
			{
				BRepTools_WireExplorer wireEx(OccWire());
				if (!wireEx.More()) return Enumerable::Empty<IXEdge^>();
				List<IXEdge^>^ edges = gcnew List<IXEdge^>();
				for (; wireEx.More(); wireEx.Next())
					edges->Add(gcnew XEdge(wireEx.Current()));
				return edges;
			}

			double XWire::Area::get()
			{
				return ShapeAnalysis::ContourArea(OccWire());
			}
			double XWire::Length::get()
			{
				BRepAdaptor_CompCurve cc(OccWire(), Standard_True);
				return cc.LastParameter() - cc.FirstParameter();
			}
		}
	}
}