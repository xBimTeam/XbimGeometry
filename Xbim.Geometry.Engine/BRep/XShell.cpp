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
			IEnumerable<IXFace^>^ XShell::Faces::get()
			{
				TopExp_Explorer faceEx(OccShell(), TopAbs_FACE);
				if (!faceEx.More()) return Enumerable::Empty<IXFace^>();
				List<IXFace^>^ faces = gcnew  List<IXFace^>();
				for (; faceEx.More(); faceEx.Next())
					faces->Add(gcnew XFace(TopoDS::Face(faceEx.Current())));
				return faces;

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