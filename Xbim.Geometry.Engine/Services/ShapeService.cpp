#include "ShapeService.h"
#include "../BRep/XbimSolid.h"
#include "../Exceptions/XbimGeometryServiceException.h"

using namespace Xbim::Geometry::BRep;
using namespace Xbim::Geometry::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			IXShape^ ShapeService::UnifyDomain(IXShape^ toFix)
			{
			
				XShapeType shapeType = toFix->ShapeType;
				switch (shapeType)
				{
				case XShapeType::Vertex:
					break;
				case XShapeType::Edge:
					break;
				case XShapeType::Wire:
					break;
				case XShapeType::Face:
					break;
				case XShapeType::Shell:
					break;
				case XShapeType::Solid:
					return gcnew XbimSolid(UnifyDomain(*(static_cast<XbimSolid^>(toFix)->Ptr())));
				case XShapeType::Compound:
					break;
				default:
					break;
				}
				throw gcnew XbimGeometryServiceException("UnifyDomain not supported for this shape type: " + shapeType.ToString());
			}
			TopoDS_Shape ShapeService::UnifyDomain(const TopoDS_Shape& toFix)
			{
				return OccHandle().UnifyDomain(toFix);
			}
			TopoDS_Solid ShapeService::UnifyDomain(const TopoDS_Solid& toFix)
			{
				return OccHandle().UnifyDomain(toFix);
			}
		}
	}
}