#pragma once

#include "./Unmanaged/NProfileFactory.h"
#include "FactoryBase.h"


namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class ProfileFactory : FactoryBase<NProfileFactory>, IXProfileService
			{	
			public:
				ProfileFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService, new NProfileFactory()) {};

				virtual IXFace^ BuildFace(IIfcProfileDef^ profileDef);
				virtual IXWire^ BuildWire(IIfcProfileDef^ profileDef);
				virtual IXEdge^ BuildEdge(IIfcProfileDef^ profileDef);
			internal:
				TopoDS_Shape BuildProfile(IIfcProfileDef^ profileDef);
			protected:
				TopoDS_Shape BuildProfile(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile);
			};
		}
	}
}

