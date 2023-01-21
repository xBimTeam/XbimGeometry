#include "ModelGeometryService.h"
#include "../Services/LoggingService.h"
#include <IMeshData_Status.hxx>
#include <IMeshTools_Parameters.hxx>
#include <BRepTools.hxx>

#include "../XbimConvert.h"
#include "../Factories/VertexFactory.h"
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
#include "../Factories/BimAuthoringToolWorkArounds.h"
#include "../Factories/MaterialFactory.h"
#include "../Factories/ProjectionFactory.h"
using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Ifc4;


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

			ModelGeometryService::ModelGeometryService(IModel^ model, ILoggerFactory^ loggerFactory)
			{
				auto logger = LoggerFactoryExtensions::CreateLogger<ModelGeometryService^>(loggerFactory);
				Dictionary<System::String^, System::Object^>^ scope = gcnew Dictionary<System::String^, System::Object^>();
				scope->Add("OriginatingSystem", model->Header->FileName->OriginatingSystem);
				scope->Add("CreatedBy", model->Header->CreatingApplication);
				scope->Add("IfcVersion", model->Header->SchemaVersion);
				logger->BeginScope(scope);
				_loggingService = gcnew Xbim::Geometry::Services::LoggingService(logger);
				SetModel(model);
				//_v5GeometryEngine = gcnew XbimGeometryCreator(model, loggerFactory);	
			}


			void ModelGeometryService::SetModel(IModel^ iModel)
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
			/// This method can be made more aware of model variations in how contexts are specified, it could be tuned to specific vendors, most now comply with the standard, so this is largely for old models
			/// </summary>
			/// <returns></returns>
			ISet<IIfcGeometricRepresentationContext^>^ ModelGeometryService::GetTypical3dContexts()
			{
				HashSet<IIfcGeometricRepresentationContext^>^ results = gcnew HashSet<IIfcGeometricRepresentationContext^>();

				for each (IIfcGeometricRepresentationSubContext ^ c in Model->Instances->OfType<IIfcGeometricRepresentationSubContext^>())
				{
					
					String^ str = c->ContextIdentifier.ToString()->ToLower() + ":" + c->ContextType.ToString()->ToLower();

					if (str->Contains("model") || str->Contains("body"))
					{
						results->Add(c);
						results->Add(c->ParentContext);
					}
				}
				if (results->Count == 0) //try the parent
				{
					for each (IIfcGeometricRepresentationContext ^ c in Model->Instances->OfType<IIfcGeometricRepresentationContext^>())
					{
						String^ str = c->ContextIdentifier.ToString()->ToLower() + ":" + c->ContextType.ToString()->ToLower();
						if (str->Contains("model") || str->Contains("body"))
							results->Add(c);
					}
				}
				return results;
			}
			IXLocation^ ModelGeometryService::Create(IIfcObjectPlacement^ placement)
			{
				throw gcnew NotImplementedException("Create() not available in Version 5");
				/*if (placement == nullptr) return gcnew XbimLocation();
				return gcnew XbimLocation(XbimConvert::ToTransform(placement, _logger));*/

			}
			IXLocation^ ModelGeometryService::CreateMappingTransform(IIfcMappedItem^ mappedItem)
			{
				throw gcnew NotImplementedException("CreateMappingTransform() not available in Version 5");
				/*auto targetTransform = XbimConvert::ToTransform(mappedItem->MappingTarget);
				auto sourceTransform = XbimConvert::ToLocation(mappedItem->MappingSource->MappingOrigin);
				auto mapLocation = sourceTransform * targetTransform;
				return gcnew XbimLocation(mapLocation);*/
			}
			System::String^ ModelGeometryService::GetBrep(const TopoDS_Shape& shape)
			{
				std::ostringstream oss;
				BRepTools::Write(shape, oss);
				return gcnew System::String(oss.str().c_str());

			}
			IXLoggingService^ ModelGeometryService::LoggingService::get()
			{
				return _loggingService;
			}

			ModelGeometryService^ ModelGeometryService::GetModelService(IModel^ model)
			{
				ModelGeometryService^ service;
				if (!Xbim::Geometry::Abstractions::Extensions::IXModelExtensions::GetTagValue<ModelGeometryService^>(model, "ModelGeometryService", service))
					throw gcnew Exception("Invalid use of Model.Tag property");
				else
					return service;
			}

			VertexFactory^ ModelGeometryService::GetVertexFactory()
			{
				if (_vertexFactory == nullptr) _vertexFactory = gcnew Xbim::Geometry::Factories::VertexFactory(this);
				return _vertexFactory;
			}
			GeometryFactory^ ModelGeometryService::GetGeometryFactory()
			{
				if (_geometryFactory == nullptr) _geometryFactory = gcnew Xbim::Geometry::Factories::GeometryFactory(this);
				return _geometryFactory;
			}

			CurveFactory^ ModelGeometryService::GetCurveFactory()
			{
				if (_curveFactory == nullptr) _curveFactory = gcnew Xbim::Geometry::Factories::CurveFactory(this);
				return _curveFactory;
			}

			SurfaceFactory^ ModelGeometryService::GetSurfaceFactory()
			{
				if (_surfaceFactory == nullptr) _surfaceFactory = gcnew Xbim::Geometry::Factories::SurfaceFactory(this);
				return _surfaceFactory;
			}


			EdgeFactory^ ModelGeometryService::GetEdgeFactory()
			{
				if (_edgeFactory == nullptr) _edgeFactory = gcnew Xbim::Geometry::Factories::EdgeFactory(this);
				return _edgeFactory;
			}

			WireFactory^ ModelGeometryService::GetWireFactory()
			{
				if (_wireFactory == nullptr) _wireFactory = gcnew Xbim::Geometry::Factories::WireFactory(this);
				return _wireFactory;
			}

			FaceFactory^ ModelGeometryService::GetFaceFactory()
			{
				if (_faceFactory == nullptr) _faceFactory = gcnew Xbim::Geometry::Factories::FaceFactory(this);
				return _faceFactory;
			}
			ShellFactory^ ModelGeometryService::GetShellFactory()
			{
				if (_shellFactory == nullptr) _shellFactory = gcnew Xbim::Geometry::Factories::ShellFactory(this);
				return _shellFactory;
			}
			SolidFactory^ ModelGeometryService::GetSolidFactory()
			{
				if (_solidFactory == nullptr) _solidFactory = gcnew Xbim::Geometry::Factories::SolidFactory(this);
				return _solidFactory;
			}
			CompoundFactory^ ModelGeometryService::GetCompoundFactory()
			{
				if (_compoundFactory == nullptr) _compoundFactory = gcnew Xbim::Geometry::Factories::CompoundFactory(this);
				return _compoundFactory;
			}
			BooleanFactory^ ModelGeometryService::GetBooleanFactory()
			{
				if (_booleanFactory == nullptr) _booleanFactory = gcnew Xbim::Geometry::Factories::BooleanFactory(this);
				return _booleanFactory;
			}

			ShapeFactory^ ModelGeometryService::GetShapeFactory()
			{
				if (_shapeFactory == nullptr) _shapeFactory = gcnew Xbim::Geometry::Factories::ShapeFactory(this);
				return _shapeFactory;
			}

			ProfileFactory^ ModelGeometryService::GetProfileFactory()
			{
				if (_profileFactory == nullptr) _profileFactory = gcnew Xbim::Geometry::Factories::ProfileFactory(this);
				return _profileFactory;
			}
			MaterialFactory^ ModelGeometryService::GetMaterialFactory()
			{
				if (_materialFactory == nullptr) _materialFactory = gcnew Xbim::Geometry::Factories::MaterialFactory(this);
				return _materialFactory;
			}
			ProjectionFactory^ ModelGeometryService::GetProjectionFactory()
			{
				if (_projectionFactory == nullptr) _projectionFactory = gcnew Xbim::Geometry::Factories::ProjectionFactory(this);
				return _projectionFactory;
			}
			Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds^ ModelGeometryService::GetBimAuthoringToolWorkArounds()
			{
				if (_bimAuthoringToolWorkArounds == nullptr) _bimAuthoringToolWorkArounds = gcnew Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds(this);
				return _bimAuthoringToolWorkArounds;
			}



			IXGeometryFactory^ ModelGeometryService::GeometryFactory::get() { return GetGeometryFactory(); }
			IXVertexFactory^ ModelGeometryService::VertexFactory::get() { return GetVertexFactory(); }
			IXCurveFactory^ ModelGeometryService::CurveFactory::get() { return GetCurveFactory(); }
			IXSurfaceFactory^ ModelGeometryService::SurfaceFactory::get() { return GetSurfaceFactory(); }
			IXEdgeFactory^ ModelGeometryService::EdgeFactory::get() { return GetEdgeFactory(); }
			IXWireFactory^ ModelGeometryService::WireFactory::get() { return GetWireFactory(); }
			IXFaceFactory^ ModelGeometryService::FaceFactory::get() { return GetFaceFactory(); }
			IXShellFactory^ ModelGeometryService::ShellFactory::get() { return GetShellFactory(); }
			IXSolidFactory^ ModelGeometryService::SolidFactory::get() { return GetSolidFactory(); }
			IXCompoundFactory^ ModelGeometryService::CompoundFactory::get() { return GetCompoundFactory(); }
			IXBooleanFactory^ ModelGeometryService::BooleanFactory::get() { return GetBooleanFactory(); }
			IXShapeFactory^ ModelGeometryService::ShapeFactory::get() { return GetShapeFactory(); }
			IXProfileFactory^ ModelGeometryService::ProfileFactory::get() { return GetProfileFactory(); }
			IXMaterialFactory^ ModelGeometryService::MaterialFactory::get() { return GetMaterialFactory(); }
			IXProjectionFactory^ ModelGeometryService::ProjectionFactory::get() { return GetProjectionFactory(); }
		}


	}
}