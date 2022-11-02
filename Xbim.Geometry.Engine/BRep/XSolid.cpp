#include "XSolid.h"
#include "XShell.h"
#include "XAxisAlignedBox.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IEnumerable<IXShell^>^ XSolid::Shells::get()
			{
				TopExp_Explorer shellEx(OccSolid(), TopAbs_SHELL);
				if (!shellEx.More()) return Enumerable::Empty<IXShell^>();
				List<IXShell^>^ shells = gcnew  List<IXShell^>();
				for (; shellEx.More(); shellEx.Next())
					shells->Add(gcnew XShell(TopoDS::Shell(shellEx.Current())));
				return shells;
			}

			
			double XSolid::Volume::get()
			{
				GProp_GProps gProps;
				BRepGProp::VolumeProperties(OccSolid(), gProps);
				return gProps.Mass();				
			}

		}
	}
}
