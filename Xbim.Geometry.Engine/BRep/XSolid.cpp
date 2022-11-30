#include "XSolid.h"
#include "XShell.h"
#include "XAxisAlignedBox.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopExp.hxx>
using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			array<IXShell^>^ XSolid::Shells::get()
			{

				TopTools_IndexedMapOfShape map;
				TopExp::MapShapes(OccSolid(), TopAbs_SHELL, map);
				array<IXShell^>^ shells = gcnew array<IXShell^>(map.Size());

				for (int i = 0; i < map.Size(); i++)
					shells[i] = (gcnew XShell(TopoDS::Shell(map(i + 1))));

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
