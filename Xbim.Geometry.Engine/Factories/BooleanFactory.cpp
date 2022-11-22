#include "BooleanFactory.h"
#include "../BRep/XEdge.h"
#include "../BRep/XSolid.h"
#include "../BRep/XFace.h"
#include "../BRep/XShell.h"
#include "../BRep/XVertex.h"
#include "../BRep/XCompound.h"
#include <TopoDS.hxx>
using namespace Xbim::Geometry::BRep;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXShape^ BooleanFactory::Build(IIfcBooleanResult^ boolResult)
			{
				TopoDS_Shape result = BuildBooleanResult(boolResult);
				if (result.IsNull() || result.NbChildren()==0)
					RaiseGeometryFactoryException("Solid result is an empty shape", boolResult);
				//find the actual type and return appropriately
				switch (result.ShapeType())
				{
				case TopAbs_COMPSOLID:
					RaiseGeometryFactoryException("Solid result type Compound Solid is not implemented", boolResult);
				case TopAbs_COMPOUND:
					return gcnew XCompound(TopoDS::Compound(result));
				case TopAbs_EDGE:
					return gcnew XEdge(TopoDS::Edge(result));
				case TopAbs_FACE:
					return gcnew XFace(TopoDS::Face(result));
				case TopAbs_SHELL:
					return gcnew XShell(TopoDS::Shell(result));
				case TopAbs_SOLID:
					return gcnew XSolid(TopoDS::Solid(result));
				case TopAbs_VERTEX:
					return gcnew XVertex(TopoDS::Vertex(result));
				default:
					RaiseGeometryFactoryException("Solid result type is not implemented", boolResult);
				}				
			}

			TopoDS_Shape BooleanFactory::BuildBooleanResult(IIfcBooleanResult^ boolResult)
			{
				TopoDS_Shape firstSolid = BuildOperand(boolResult->FirstOperand);
				TopoDS_Shape secondSolid = BuildOperand(boolResult->SecondOperand);
				
				switch (boolResult->Operator)
				{
				case IfcBooleanOperator::UNION:
					return _shapeFactory->NUnifyDomain(Ptr()->Union(firstSolid, secondSolid, _modelService->MinimumGap));					
				case IfcBooleanOperator::DIFFERENCE:
					return _shapeFactory->NUnifyDomain(Ptr()->Cut(firstSolid, secondSolid, _modelService->MinimumGap));
				case IfcBooleanOperator::INTERSECTION:
					return _shapeFactory->NUnifyDomain(Ptr()->Intersect(firstSolid, secondSolid, _modelService->MinimumGap));
				default:
					RaiseGeometryFactoryException("Not implemented. BooleanOperation type: " + boolResult->Operator.ToString(), boolResult);
					//break;
				}
			}

			TopoDS_Shape BooleanFactory::BuildOperand(IIfcBooleanOperand^ boolOp)
			{
			
				IIfcSolidModel^ solidModel = dynamic_cast<IIfcSolidModel^>(boolOp);
				if (solidModel != nullptr) return _solidFactory->BuildSolidModel(solidModel);
				IIfcHalfSpaceSolid^ halfSpace = dynamic_cast<IIfcHalfSpaceSolid^>(boolOp);
				if (halfSpace != nullptr) return BuildHalfSpace(halfSpace); //not really a solid, do it in this factory
				IIfcCsgPrimitive3D^ csgPrim = dynamic_cast<IIfcCsgPrimitive3D^>(boolOp);
				if (csgPrim != nullptr) return _solidFactory->BuildCsgPrimitive3D(csgPrim);
				IIfcBooleanResult^ boolRes = dynamic_cast<IIfcBooleanResult^>(boolOp);
				if (boolRes != nullptr) return BuildBooleanResult(boolRes);				
				RaiseGeometryFactoryException("Not implemented. BooleanOperand type", boolOp);
				return TopoDS_Solid();
			}

			TopoDS_Solid BooleanFactory::BuildHalfSpace(IIfcHalfSpaceSolid^ halfSpace)
			{
				return TopoDS_Solid();
			}

		}
	}
}