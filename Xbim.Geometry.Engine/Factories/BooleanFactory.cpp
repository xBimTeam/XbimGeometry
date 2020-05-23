#include "BooleanFactory.h"
#include "../BRep/XbimEdge.h"
#include "../BRep/XbimSolid.h"
#include "../BRep/XbimFace.h"
#include "../BRep/XbimShell.h"
#include "../BRep/XbimVertex.h"
#include "../BRep/XbimCompound.h"
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
					throw gcnew XbimGeometryFactoryException("Solid result is an empty shape");
				//find the actual type and return appropriately
				switch (result.ShapeType())
				{
				case TopAbs_COMPSOLID:
					throw gcnew XbimGeometryFactoryException("Solid result type Compound Solid is not implemented");
				case TopAbs_COMPOUND:
					return gcnew XbimCompound(TopoDS::Compound(result));
				case TopAbs_EDGE:
					return gcnew XbimEdge(TopoDS::Edge(result));
				case TopAbs_FACE:
					return gcnew XbimFace(TopoDS::Face(result));
				case TopAbs_SHELL:
					return gcnew XbimShell(TopoDS::Shell(result));
				case TopAbs_SOLID:
					return gcnew XbimSolid(TopoDS::Solid(result));
				case TopAbs_VERTEX:
					return gcnew XbimVertex(TopoDS::Vertex(result));
				default:
					throw gcnew XbimGeometryFactoryException("Solid result type is not implemented");
				}				
			}

			TopoDS_Shape BooleanFactory::BuildBooleanResult(IIfcBooleanResult^ boolResult)
			{
				TopoDS_Shape firstSolid = BuildOperand(boolResult->FirstOperand);
				TopoDS_Shape secondSolid = BuildOperand(boolResult->SecondOperand);
				
				switch (boolResult->Operator)
				{
				case IfcBooleanOperator::UNION:
					return _shapeService->UnifyDomain(Ptr()->Union(firstSolid, secondSolid));					
				case IfcBooleanOperator::DIFFERENCE:
					return _shapeService->UnifyDomain(Ptr()->Cut(firstSolid, secondSolid));
				case IfcBooleanOperator::INTERSECTION:
					return _shapeService->UnifyDomain(Ptr()->Intersect(firstSolid, secondSolid));
				default:
					throw gcnew XbimGeometryFactoryException("Not implemented. BooleanOperation type: " + boolResult->Operator.ToString());
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
			
				//case XBooleanOperandType::IfcTessellatedFaceSet:					
				throw gcnew XbimGeometryFactoryException("Not implemented. BooleanOperand type: " + boolOp->GetType()->Name);
			}

			TopoDS_Solid BooleanFactory::BuildHalfSpace(IIfcHalfSpaceSolid^ halfSpace)
			{
				return TopoDS_Solid();
			}

		}
	}
}