#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimAxisAlignedBox.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IEnumerable<IXShell^>^ XbimSolid::Shells::get()
			{
				TopExp_Explorer shellEx(OccHandle(), TopAbs_SHELL);
				if (!shellEx.More()) return Enumerable::Empty<IXShell^>();
				List<IXShell^>^ shells = gcnew  List<IXShell^>();
				for (; shellEx.More(); shellEx.Next())
					shells->Add(gcnew XbimShell(TopoDS::Shell(shellEx.Current())));
				return shells;
			}

			
		}
	}
}
