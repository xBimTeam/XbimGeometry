#include "XbimShell.h"
#include "XbimFace.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include "XbimFace.h"
using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IEnumerable<IXFace^>^ XbimShell::Faces::get()
			{
				TopExp_Explorer faceEx(OccHandle(), TopAbs_FACE);
				if (!faceEx.More()) return Enumerable::Empty<IXFace^>();
				List<IXFace^>^ faces = gcnew  List<IXFace^>();
				for (; faceEx.More(); faceEx.Next())
					faces->Add(gcnew XbimFace(TopoDS::Face(faceEx.Current())));
				return faces;

			}
		}
	}
}