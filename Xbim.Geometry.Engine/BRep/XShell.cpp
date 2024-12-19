#include "XShell.h"
#include "XFace.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include "XFace.h"
#include <GProp_PGProps.hxx>
#include <BRepGProp.hxx>
using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			array<IXFace^>^ XShell::Faces::get()
			{
				TopExp_Explorer faceEx(OccShell(), TopAbs_FACE);
				TopoDS_ListOfShape shapes;
				for (; faceEx.More(); faceEx.Next())
					shapes.Append(faceEx.Current());

				array<IXFace^>^ managedShapes = gcnew  array<IXFace^>(shapes.Size());
				int i = 0;
				for(auto&& shape:shapes)
					managedShapes[i++]= gcnew XFace(TopoDS::Face(shape));
				return managedShapes;
			}
			double XShell::SurfaceArea::get()
			{
				GProp_GProps gProps;
				BRepGProp::SurfaceProperties(OccShell(), gProps);
				return gProps.Mass();
			}
		}
	}
}