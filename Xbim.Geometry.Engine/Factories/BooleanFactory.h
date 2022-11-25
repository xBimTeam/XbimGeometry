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
				BooleanFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService,new NBooleanFactory()){	}
				virtual IXShape^ Build(IIfcBooleanResult^ boolResult);
				TopoDS_Shape BuildBooleanResult(IIfcBooleanResult^ boolResult);
				TopoDS_Shape BuildOperand(IIfcBooleanOperand^ boolOp);
				TopoDS_Solid BuildHalfSpace(IIfcHalfSpaceSolid^ halfSpace);
			};
		}
	}
}

