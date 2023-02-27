#pragma once

#include "./Unmanaged/NBooleanFactory.h"
#include "FactoryBase.h"
#include "SolidFactory.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class BooleanFactory : FactoryBase<NBooleanFactory>, IXBooleanFactory
			{
			private:
				
			public:
				BooleanFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService,new NBooleanFactory()){	}
				virtual IXShape^ Build(IIfcBooleanResult^ boolResult);
				TopoDS_Shape BuildBooleanResult(IIfcBooleanResult^ boolResult);
				TopoDS_Shape BuildOperand(IIfcBooleanOperand^ boolOp);
				TopoDS_Solid BuildHalfSpace(IIfcHalfSpaceSolid^ halfSpace);

#pragma region Native calls
				/// <summary>
				/// Cuts a set of tools from a set of arguments, default model tolerance is used
				/// </summary>
				/// <param name="arguments">body of the shape</param>
				/// <param name="tools">shapes to cut from the body</param>
				/// <returns>The result or an empty shape if the boolean fails</returns>
				TopoDS_Shape Cut(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools);
				/// <summary>
				/// Cuts a set of tools from a set of arguments
				/// </summary>
				/// <param name="arguments">body of the shape</param>
				/// <param name="tools">shapes to cut from the body</param>
				/// <param name="tolerance">overrides the default model tolerance</param>
				/// <returns>The result or an empty shape if the boolean fails</returns>
				TopoDS_Shape Cut(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools, double tolerance);
				/// <summary>
				/// Unions a set of tools with a set of arguments, default model tolerance is used
				/// </summary>
				/// <param name="arguments">body of the shape</param>
				/// <param name="tools">shapes to union with the body</param>
				/// <returns>The result or an empty shape if the boolean fails</returns>
				TopoDS_Shape Union(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools);
				/// <summary>
				/// Unions a set of tools with a set of arguments
				/// </summary>
				/// <param name="arguments">body of the shape</param>
				/// <param name="tools">shapes to union with the body</param>
				/// <param name="tolerance">overrides the default model tolerance</param>
				/// <returns>The result or an empty shape if the boolean fails</returns>
				TopoDS_Shape Union(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools, double tolerance);
				/// <summary>
				/// Intersects a set of tools with a set of arguments, default model tolerance is used
				/// </summary>
				/// <param name="arguments">body of the shape</param>
				/// <param name="tools">shapes to intersect with the body</param>
				/// <returns>The result or an empty shape if the boolean fails</returns>
				TopoDS_Shape Intersect(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools);
				/// <summary>
				/// Intersects a set of tools with a set of arguments
				/// </summary>
				/// <param name="arguments">body of the shape</param>
				/// <param name="tools">shapes to intersect with the body</param>
				/// <param name="tolerance">overrides the default model tolerance</param>
				/// <returns>The result or an empty shape if the boolean fails</returns>
				TopoDS_Shape Intersect(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools, double tolerance);
#pragma endregion

			};
		}
	}
}

