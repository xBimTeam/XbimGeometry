#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include <BRepTools.hxx>
#include <Bnd_Box.hxx>

#include "XbimAxisAlignedBox.h"
#include <BRepBndLib.hxx>
using namespace System;
using namespace System::Collections::Generic;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			template <typename TShape>
			public ref class XbimShape abstract : XbimHandle<TShape>, IXShape
			{
			protected:
				XbimShape(TShape* pShape) : XbimHandle(pShape) {};
			public:
				virtual property XShapeType ShapeType {XShapeType get() abstract; };
				virtual property String^ Json {String^ get()
				{
					std::ostringstream oss;
					OccHandle().DumpJson(oss);
					return gcnew String(oss.str().c_str());
				}};

				virtual property String^ Brep {String^ get()
				{
					std::ostringstream oss;
					oss << "DBRep_DrawableShape" << std::endl;
					BRepTools::Write(OccHandle(), oss);
					return gcnew String(oss.str().c_str());
				}};

				virtual IXAxisAlignedBoundingBox^ Bounds()
				{
					Bnd_Box* pBox = new Bnd_Box();
					BRepBndLib::Add(OccHandle(), *pBox, false);

					return gcnew XbimAxisAlignedBox(pBox);
				};

			};
		}
	}
}

