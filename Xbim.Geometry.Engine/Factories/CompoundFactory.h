#pragma once
#include "FactoryBase.h"
#include "./Unmanaged/NCompoundFactory.h"



namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class CompoundFactory : public FactoryBase<NCompoundFactory>, IXCompoundFactory
			{

			public:
				
				virtual IXCompound^ CreateEmpty();
				virtual IXCompound^ CreateFrom(System::Collections::Generic::IEnumerable<IXShape^>^ shapes);
				
				CompoundFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NCompoundFactory()) {}

			};
		}
	}
}