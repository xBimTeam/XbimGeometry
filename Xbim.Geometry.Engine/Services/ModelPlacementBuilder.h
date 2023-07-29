#pragma once

#include "../Factories/FactoryBase.h"
#include "../Factories/Unmanaged/NGeometryFactory.h"
#include "ModelGeometryService.h"
#include "../BRep/XLocation.h"

#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

using namespace Xbim::Common;
using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Geometry::Factories;
using namespace Xbim::Geometry::BRep;


namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			public ref class ModelPlacementBuilder : public FactoryBase<NGeometryFactory>, IXModelPlacementBuilder
			{
			private:
				bool _adjustWcs; 
				Xbim::Geometry::Services::ModelGeometryService^ _modelGeometryServices;
				IXLocation^ _wcs;
				int _rootId = -1;

			public:
				ModelPlacementBuilder(Xbim::Geometry::Services::ModelGeometryService^ modelGeometryService) : FactoryBase(modelGeometryService, new NGeometryFactory())
				{
					_modelGeometryServices = modelGeometryService;
					_wcs = gcnew XLocation();
					
					List<IIfcLocalPlacement^>^ localPlacements = Enumerable::ToList(_modelGeometryServices->Model->Instances->OfType<IIfcLocalPlacement^>());

					if(localPlacements->Count == 0) return; //nothing to do

					List<IIfcLocalPlacement^>^ roots = gcnew List<IIfcLocalPlacement^>();

					// identify tree roots
					for each(IIfcLocalPlacement^ localPlacement in localPlacements)
					{
						if (localPlacement->PlacementRelTo == nullptr)
						{ 
							roots->Add(localPlacement);
							if (roots->Count > 1)
								return; // more than one root, no need to continue
						} 
					}

					if (roots->Count == 1)
					{
						IIfcLocalPlacement^ root = roots[0];
						_rootId = root->EntityLabel;
						_wcs = _modelGeometryServices->GeometryFactory->BuildLocation(root);
					}
				};

				virtual property IXLocation^ WorldCoordinateSystem { 
					IXLocation^ get() { return _wcs;  };
				}

				virtual IXLocation^ BuildLocation(IIfcObjectPlacement^ placement, bool adjustWcs);
			
				gp_Trsf ToTransform(IIfcLocalPlacement^ localPlacement, bool adjustWcs);
				bool BuildDirection3d(IIfcDirection^ ifcDir, gp_Vec& dir);
				gp_Pnt BuildPoint3d(IIfcCartesianPoint^ ifcPoint);

			};
		}
	}
}

