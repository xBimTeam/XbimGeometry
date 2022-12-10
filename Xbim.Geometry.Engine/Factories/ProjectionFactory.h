#pragma once
#include "FactoryBase.h"

#include "Unmanaged/NProjectionFactory.h"

using namespace Microsoft::Extensions::Logging;
using namespace Microsoft::Extensions::Logging::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class ProjectionFactory : public FactoryBase<NProjectionFactory>, IXProjectionFactory
			{
			
			public:
				ProjectionFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NProjectionFactory()){}
				
				virtual IXFootprint^ CreateFootprint(IXShape^ shape, double linearDeflection, double angularDeflection);
				virtual IXFootprint^ CreateFootprint(IXShape^ shape);
				virtual IXCompound^ GetOutline(IXShape^ shape);
				virtual IEnumerable<IXFace^>^ CreateSection(IXShape^ shape, IXPlane^ cutPlane); 
			};
		}
	}
}
