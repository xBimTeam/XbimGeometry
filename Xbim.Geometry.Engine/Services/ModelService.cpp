#include "ModelService.h"
#include "../Services/LoggingService.h"
#include <IMeshData_Status.hxx>
#include <IMeshTools_Parameters.hxx>

#include "../XbimConvert.h"
#include "../Factories/GeometryFactory.h"
#include "../Factories/CurveFactory.h"
#include "../Factories/SurfaceFactory.h"
#include "../Factories/EdgeFactory.h"
#include "../Factories/WireFactory.h"
#include "../Factories/FaceFactory.h"
#include "../Factories/ShellFactory.h"
#include "../Factories/SolidFactory.h"
#include "../Factories/CompoundFactory.h"
#include "../Factories/BooleanFactory.h"
#include "../Factories/ProfileFactory.h"
using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Ifc4;
//using namespace Xbim::Geometry::BRep;

using namespace System;
using namespace Xbim::IO::Memory;
using namespace Xbim::Geometry::Factories;
using namespace Xbim::Geometry::Services;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{

			ModelService::ModelService(IModel^ iModel, ILogger^ logger)
			{
				_loggingService = gcnew Xbim::Geometry::Services::LoggingService(logger);
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
					minimumGap = iModel->ModelFactors->OneMilliMeter / 5; //0.2 of a millimeter, ok for most metric models
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

			GeometryFactory^ ModelService::GetGeometryFactory()
			{
				if (_geometryFactory == nullptr) _geometryFactory = gcnew Xbim::Geometry::Factories::GeometryFactory(this);
				return _geometryFactory;
			}

			CurveFactory^ ModelService::GetCurveFactory()
			{
				if (_curveFactory == nullptr) _curveFactory = gcnew Xbim::Geometry::Factories::CurveFactory(this);
				return _curveFactory;
			}

			SurfaceFactory^ ModelService::GetSurfaceFactory()
			{
				if (_surfaceFactory == nullptr) _surfaceFactory = gcnew Xbim::Geometry::Factories::SurfaceFactory(this);
				return _surfaceFactory;
			}


			EdgeFactory^ ModelService::GetEdgeFactory()
			{
				if (_edgeFactory == nullptr) _edgeFactory = gcnew Xbim::Geometry::Factories::EdgeFactory(this);
				return _edgeFactory;
			}

			WireFactory^ ModelService::GetWireFactory()
			{
				if (_wireFactory == nullptr) _wireFactory = gcnew Xbim::Geometry::Factories::WireFactory(this);
				return _wireFactory;
			}

			FaceFactory^ ModelService::GetFaceFactory()
			{
				if (_faceFactory == nullptr) _faceFactory = gcnew Xbim::Geometry::Factories::FaceFactory(this);
				return _faceFactory;
			}
			ShellFactory^ ModelService::GetShellFactory()
			{
				if (_shellFactory == nullptr) _shellFactory = gcnew Xbim::Geometry::Factories::ShellFactory(this);
				return _shellFactory;
			}
			SolidFactory^ ModelService::GetSolidFactory()
			{
				if (_solidFactory == nullptr) _solidFactory = gcnew Xbim::Geometry::Factories::SolidFactory(this);
				return _solidFactory;
			}
			CompoundFactory^ ModelService::GetCompoundFactory()
			{
				if (_compoundFactory == nullptr) _compoundFactory = gcnew Xbim::Geometry::Factories::CompoundFactory(this);
				return _compoundFactory;
			}
			BooleanFactory^ ModelService::GetBooleanFactory()
			{
				if (_booleanFactory == nullptr) _booleanFactory = gcnew Xbim::Geometry::Factories::BooleanFactory(this);
				return _booleanFactory;
			}

			ShapeFactory^ ModelService::GetShapeFactory()
			{
				if (_shapeFactory == nullptr) _shapeFactory = gcnew Xbim::Geometry::Factories::ShapeFactory(this);
				return _shapeFactory;
			}

			ProfileFactory^ ModelService::GetProfileFactory()
			{
				if (_profileFactory == nullptr) _profileFactory = gcnew Xbim::Geometry::Factories::ProfileFactory(this);
				return _profileFactory;
			}


			IXGeometryFactory^ ModelService::GeometryFactory::get() { return GetGeometryFactory(); }
			IXCurveFactory^ ModelService::CurveFactory::get() { return GetCurveFactory(); }
			IXSurfaceFactory^ ModelService::SurfaceFactory::get() { return GetSurfaceFactory(); }
			IXEdgeFactory^ ModelService::EdgeFactory::get() { return GetEdgeFactory(); }
			IXWireFactory^ ModelService::WireFactory::get() { return GetWireFactory(); }
			IXFaceFactory^ ModelService::FaceFactory::get() { return GetFaceFactory(); }
			IXShellFactory^ ModelService::ShellFactory::get() { return GetShellFactory(); }
			IXSolidFactory^ ModelService::SolidFactory::get() { return GetSolidFactory(); }
			IXCompoundFactory^ ModelService::CompoundFactory::get() { return GetCompoundFactory(); }
			IXBooleanFactory^ ModelService::BooleanFactory::get() { return GetBooleanFactory(); }
			IXShapeFactory^ ModelService::ShapeFactory::get() { return GetShapeFactory(); }
			IXProfileFactory^ ModelService::ProfileFactory::get() { return GetProfileFactory(); }
		}


	}
}