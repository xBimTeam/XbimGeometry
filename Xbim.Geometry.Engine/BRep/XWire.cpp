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
			
			array<IXEdge^>^ XWire::EdgeLoop::get()
			{
				BRepTools_WireExplorer wireEx(OccWire());
				TopoDS_ListOfShape shapes;
				for (; wireEx.More(); wireEx.Next())
					shapes.Append(wireEx.Current());

				array<IXEdge^>^ managedShapes = gcnew  array<IXEdge^>(shapes.Size());
				int i = 0;
				for (auto&& shape : shapes)
					managedShapes[i++] = gcnew XEdge(TopoDS::Edge(shape));
				return managedShapes;

			}

			double XWire::ContourArea::get()
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