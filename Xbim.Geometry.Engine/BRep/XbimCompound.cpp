#include "XbimCompound.h"
#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimFace.h"
#include <TopoDS_Iterator.hxx>
#include <TopoDS.hxx>
#include <BRepTools.hxx>

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			String^ XbimCompound::BRepDefinition()
			{
				std::ostringstream oss;
				oss << "DBRep_DrawableShape" << std::endl;
				BRepTools::Write(OccHandle(), oss);
				return gcnew String(oss.str().c_str());
			}


			bool XbimCompound::IsSolidsOnly::get()
			{				
				bool isSolidsOnly = false;
				for (TopoDS_Iterator iter(OccHandle(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_SOLID)
						return false;
					else
						isSolidsOnly = true; //we have a solid
				};
				return isSolidsOnly;
			};


			bool XbimCompound::IsShellsOnly::get()
			{
				
				bool isShellsOnly = false;
				for (TopoDS_Iterator iter(OccHandle(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_SHELL)
						return false;
					else
						isShellsOnly = true; //we have a shell					
				};
				return isShellsOnly;
			};
			bool XbimCompound::IsFacesOnly::get()
			{
				
				bool isFacesOnly = false;
				for (TopoDS_Iterator iter(OccHandle(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_FACE)
						return false;
					else
						isFacesOnly = true; //we have a shell					
				};
				return isFacesOnly;
			};
			bool XbimCompound::HasSolids::get()
			{
				for (TopoDS_Iterator iter(OccHandle(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SOLID) return true;					
				}
				return false;
			};
			bool XbimCompound::HasShells::get()
			{
				for (TopoDS_Iterator iter(OccHandle(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SHELL) return true;					
				}
				return false;
			};

			bool XbimCompound::HasFaces::get()
			{
				for (TopoDS_Iterator iter(OccHandle(),false,false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_FACE) return true;					
				}
				return false;
			};

			IEnumerable<IXSolid^>^ XbimCompound::Solids::get()
			{
				
				List<IXSolid^>^ solids = gcnew List<IXSolid^>();
				for (TopoDS_Iterator iter(OccHandle()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SOLID)
						solids->Add(gcnew XbimSolid(TopoDS::Solid(iter.Value())));
				}
				return solids;
			};

			IEnumerable<IXShell^>^ XbimCompound::Shells::get()
			{

				List<IXShell^>^ shells = gcnew List<IXShell^>();
				for (TopoDS_Iterator iter(OccHandle()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SHELL)
						shells->Add(gcnew XbimShell(TopoDS::Shell(iter.Value())));
				}
				return shells;
			};

			IEnumerable<IXFace^>^ XbimCompound::Faces::get()
			{
				List<IXFace^>^ faces = gcnew List<IXFace^>();
				for (TopoDS_Iterator iter(OccHandle()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_FACE) faces->Add(gcnew XbimFace(TopoDS::Face(iter.Value())));
				}
				return faces;
			};
		}
	}
}