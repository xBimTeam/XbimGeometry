
#include <BRepTools.hxx>
#include "XbimGeometryObject.h"
#include "XbimOccShape.h"

#include "XBimCompound.h"
#include "XBimSolid.h"
#include "XBimShell.h"
#include "XBimFace.h"
#include "XBimWire.h"
#include "XBimEdge.h"
#include "XBimVertex.h"

#include "XbimGeometryObjectSet.h"
#include "XBimSolidSet.h"
#include "XBimShellSet.h"
#include "XBimFaceSet.h"
#include "XBimWireSet.h"
#include "XBimEdgeSet.h"
#include "XBimVertexSet.h"

#include "./Brep/XCompound.h"
#include "./Brep/XSolid.h"
#include "./Brep/XShell.h"
#include "./Brep/XFace.h"
#include "./Brep/XWire.h"
#include "./Brep/XEdge.h"
#include "./Brep/XVertex.h"

using namespace Xbim::Geometry::BRep;
namespace Xbim
{
	namespace Geometry
	{


		System::String^ XbimGeometryObject::ToBRep::get()
		{
			if (!IsValid)
				return System::String::Empty;
			std::ostringstream oss;
			if (dynamic_cast<XbimOccShape^>(this))
			{
				oss << "DBRep_DrawableShape" << std::endl;
				BRepTools::Write((XbimOccShape^)this, oss);
				return gcnew System::String(oss.str().c_str());
			}


			//otherwise we don't do it
			return System::String::Empty;
		}
		IXShape^ XbimGeometryObject::ToXShape(IXbimGeometryObject^ geomObj)
		{
			if (geomObj == nullptr || !geomObj->IsValid)
				return nullptr;
			switch (geomObj->GeometryType)
			{
			case XbimGeometryObjectType::XbimGeometryObjectSetType:
				return static_cast<XbimGeometryObjectSet^>(geomObj)->ToXCompound();
			case XbimGeometryObjectType::XbimSolidType:
				return gcnew XSolid(static_cast<XbimSolid^>(geomObj));
			case XbimGeometryObjectType::XbimSolidSetType:
				return static_cast<XbimSolidSet^>(geomObj)->ToXCompound();
			case XbimGeometryObjectType::XbimShellType:
				return gcnew XShell(static_cast<XbimShell^>(geomObj));
			case XbimGeometryObjectType::XbimShellSetType:
				return static_cast<XbimShellSet^>(geomObj)->ToXCompound();
			case XbimGeometryObjectType::XbimFaceType:
				return gcnew XFace(static_cast<XbimFace^>(geomObj));
			case XbimGeometryObjectType::XbimFaceSetType:
				return static_cast<XbimFaceSet^>(geomObj)->ToXCompound();
			case XbimGeometryObjectType::XbimEdgeType:
				return gcnew XEdge(static_cast<XbimEdge^>(geomObj));
			case XbimGeometryObjectType::XbimEdgeSetType:
				return static_cast<XbimEdgeSet^>(geomObj)->ToXCompound();
			case XbimGeometryObjectType::XbimVertexType:
				return gcnew XVertex(static_cast<XbimVertex^>(geomObj));
			case XbimGeometryObjectType::XbimVertexSetType:
				return static_cast<XbimVertexSet^>(geomObj)->ToXCompound();
			case XbimGeometryObjectType::XbimWireType:
				return gcnew XWire(static_cast<XbimWire^>(geomObj));
			case XbimGeometryObjectType::XbimWireSetType:
				return static_cast<XbimWireSet^>(geomObj)->ToXCompound();
			case XbimGeometryObjectType::XbimCompoundType:
				return gcnew XCompound(static_cast<XbimCompound^>(geomObj));
			case XbimGeometryObjectType::XbimPointType:
				break;
			case XbimGeometryObjectType::XbimCurveType:
				break;
			case XbimGeometryObjectType::XbimCurveSetType:
				break;
			case XbimGeometryObjectType::XbimMeshType:
				break;
			case XbimGeometryObjectType::XbimMeshSetType:
				break;
			default:
				break;
			}
			throw gcnew XbimGeometryServiceException("Unsupported geometry type");
		}
	}
}
