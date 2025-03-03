#pragma once

#include <TopoDS_Solid.hxx>
#include <BRepTools.hxx>
#include <Bnd_Box.hxx>
#include <IMeshTools_Parameters.hxx>
#include <BRepBndLib.hxx>

#include "../XbimHandle.h"
#include "XAxisAlignedBox.h"


#include "../BRep/XLocation.h"



#define TOPO_SHAPE(shape) static_cast<XShape^>(shape)->GetTopoShape()

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			using namespace System::Collections::Generic;
			using namespace Xbim::Geometry::Abstractions;
			using namespace Xbim::Geometry::Exceptions;

			ref class XCompound;
			ref class XSolid;
			ref class XShell;
			ref class XFace;
			ref class XWire;
			ref class XEdge;
			ref class XVertex;
			ref class XSectionedSurface;

			public ref class XShape abstract : XbimHandle<TopoDS_Shape>, IXShape
			{
			protected:
				XShape(TopoDS_Shape* pShape) : XbimHandle(pShape) {};

			public:

				virtual property XShapeType ShapeType {XShapeType get() abstract; };


				/*virtual property System::String^ Json {System::String^ get()
				{
					std::ostringstream oss;
					OccHandle().DumpJson(oss);
					return gcnew System::String(oss.str().c_str());
				}};*/

				virtual System::String^ BrepString()
				{
					std::ostringstream oss;
					oss << "DBRep_DrawableShape" << std::endl;
					BRepTools::Write(OccHandle(), oss);
					return gcnew System::String(oss.str().c_str());
				};


				virtual IXAxisAlignedBoundingBox^ Bounds()
				{
					Bnd_Box* pBox = new Bnd_Box();
					BRepBndLib::Add(OccHandle(), *pBox, false);

					return gcnew XAxisAlignedBox(pBox);
				};
				virtual bool IsEmptyShape()
				{
					return OccHandle().NbChildren() == 0;
				}

				virtual bool IsValidShape();

				virtual property bool IsClosed {bool get() {
					return OccHandle().Closed();
				}
				};

				virtual  bool Triangulate(IXMeshFactors^ meshFactors);

				virtual property IXLocation^ Location { IXLocation^ get() { return gcnew XLocation(OccHandle().Location()); } };
				virtual System::Collections::Generic::IEnumerable<IXFace^>^ AllFaces();
				virtual const TopoDS_Shape& GetTopoShape() { return *(this->Ptr()); };
				virtual bool IsEqual(IXShape^ shape);
				virtual int ShapeHashCode() { return (int)std::hash<TopoDS_Shape>()(OccHandle()); }
				virtual property XOrientation Orientation { XOrientation get() 
				{
					TopAbs_Orientation orientation = OccHandle().Orientation();
					switch (orientation)
					{
					case TopAbs_FORWARD:
						return XOrientation::Forward;
					case TopAbs_REVERSED:
						return XOrientation::Reversed;
					case TopAbs_INTERNAL:
						return XOrientation::Internal;
					case TopAbs_EXTERNAL:
						return XOrientation::External;
					default:
						throw gcnew System::ArgumentException("Unsupported Oreintation");
					}
				}};
				
				static IXShape^ GetXbimShape(const TopoDS_Shape& shape);
				

			};




		}
	}
}

