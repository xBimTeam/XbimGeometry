#include "ModelService.h"

#include <IMeshData_Status.hxx>
#include <IMeshTools_Parameters.hxx>

#include "../XbimConvert.h"
//#include "../BRep/XbimLocation.h"

using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Ifc4;
//using namespace Xbim::Geometry::BRep;

using namespace System;
using namespace Xbim::IO::Memory;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{

		
			/// MinArea is 25 sq millimetres
			
			
			ModelService::ModelService(IModel^ iModel)
			{
				SetModel(iModel);
			}
			
			
			void ModelService::SetModel(IModel^ iModel)
			{
				model = iModel;
				precisionSquared = model->ModelFactors->Precision * model->ModelFactors->Precision;			
				model->Tag = this; //support for legacy engine lookup
				
				minAreaM2 = std::pow(0.002, 2) * std::pow(model->ModelFactors->OneMeter, 2)/*2 mm x 2mm*/;

				//here we can consider specific authoring tools
				//Revit has a minimum length of a line = 1 inch / 32 = 0.79375mm
				//therefore for certain operations we need to set the minimum gap 
				IIfcApplication^ anApp = Enumerable::FirstOrDefault(iModel->Instances->OfType<IIfcApplication^>());

				if (anApp != nullptr && anApp->ApplicationIdentifier.ToString() == "Revit")
					minimumGap = (iModel->ModelFactors->OneMilliMeter * 25.4) / 32; //1/32nd of an inch, revits min gap
				else
					minimumGap = iModel->ModelFactors->OneMilliMeter / 10; //1/10 of a millimeter, ok for most metric models
			}

			/// <summary>
			/// This method can be made more aware of model variations in how contexts are specified
			/// </summary>
			/// <returns></returns>
			ISet<IIfcGeometricRepresentationContext^>^ ModelService::GetTypical3dContexts()
			{
				HashSet<IIfcGeometricRepresentationContext^>^ results = gcnew HashSet<IIfcGeometricRepresentationContext^>();

				for each (IIfcGeometricRepresentationSubContext ^ c in Model->Instances->OfType<IIfcGeometricRepresentationSubContext^>())
				{
					if ((String::Compare(c->ContextIdentifier.ToString(), "body", true) == 0 || String::IsNullOrWhiteSpace(c->ContextIdentifier.ToString()))
						&& String::Compare(c->ContextType.ToString(), "model", true) == 0)
					{
						results->Add(c);
						results->Add(c->ParentContext);
					}
				}
				if (results->Count == 0) //try the parent
				{
					for each (IIfcGeometricRepresentationContext ^ c in Model->Instances->OfType<IIfcGeometricRepresentationContext^>())
					{
						if ((String::Compare(c->ContextIdentifier.ToString(), "body", true) == 0 || String::IsNullOrWhiteSpace(c->ContextIdentifier.ToString()))
							&& String::Compare(c->ContextType.ToString(), "model", true) == 0)
							results->Add(c);
					}
				}
				return results;
			}
			IXLocation^ ModelService::Create(IIfcObjectPlacement^ placement)
			{
				throw gcnew NotImplementedException("Create() not available in Version 5");
				/*if (placement == nullptr) return gcnew XbimLocation();
				return gcnew XbimLocation(XbimConvert::ToTransform(placement, _logger));*/
			
			}
			IXLocation^ ModelService::CreateMappingTransform(IIfcMappedItem^ mappedItem)
			{
				throw gcnew NotImplementedException("CreateMappingTransform() not available in Version 5");
				/*auto targetTransform = XbimConvert::ToTransform(mappedItem->MappingTarget);
				auto sourceTransform = XbimConvert::ToLocation(mappedItem->MappingSource->MappingOrigin);
				auto mapLocation = sourceTransform * targetTransform;
				return gcnew XbimLocation(mapLocation);*/
			}
		}


	}
}