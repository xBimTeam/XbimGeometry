#pragma once

#include "../Factories/FactoryBase.h"
#include "../Factories/Unmanaged/NGeometryFactory.h"
#include "ModelGeometryService.h"
#include "../BRep/XPoint.h"

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
				Xbim::Geometry::Services::ModelGeometryService^ _modelGeometryServices;
				IXPoint^ _wcs;
				IXLocation^ _rootTrsf;
				int _rootId = -1;

			public:
				ModelPlacementBuilder(Xbim::Geometry::Services::ModelGeometryService^ modelGeometryService) : FactoryBase(modelGeometryService, new NGeometryFactory())
				{
					_modelGeometryServices = modelGeometryService;
					_wcs = gcnew XPoint(0, 0, 0);
					
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
						_rootTrsf = _modelGeometryServices->GeometryFactory->BuildLocation(root);
						_wcs = _rootTrsf->Translation;
						_rootTrsf->SetTranslation(0, 0, 0);
					}
				};

				virtual property IXPoint^ WorldCoordinateSystem { 
					IXPoint^ get() { return _wcs;  };
				}

				virtual property IXLocation^ RootPlacement { 
					IXLocation^ get() { return _rootTrsf;  };
				}

				virtual IXLocation^ BuildLocation(IIfcObjectPlacement^ placement, bool adjustWcs);
			
				gp_Trsf ToTransform(IIfcLocalPlacement^ localPlacement, bool adjustWcs);
				bool BuildDirection3d(IIfcDirection^ ifcDir, gp_Vec& dir);
				gp_Pnt BuildPoint3d(IIfcCartesianPoint^ ifcPoint);

			};
		}
	}
}

