#pragma once

#include "Unmanaged/NEdgeFactory.h"
#include "FactoryBase.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class EdgeFactory : FactoryBase<NEdgeFactory>, IXEdgeFactory
			{


#pragma region IX..... interface methods. exposed publicly		

			public:
				virtual IXEdge^ BuildEdge(IXPoint^ start, IXPoint^ end);
				virtual IXEdge^ Build(IIfcCurve^ curve);
				virtual IXEdge^ BuildEdge(IXCurve^ curve);

#pragma endregion


#pragma region  Methods returning Opencascade native types, internal use only

			internal:
				TopoDS_Edge BuildCurve(IIfcCurve^ curve);

#pragma endregion

#pragma region Constructors and private and protected data

			private:

			public:
				EdgeFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService, new NEdgeFactory()) {}
#pragma endregion		
			};
		}
	}
}

