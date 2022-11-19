#pragma once
#include "../XbimHandle.h"
#include "./Unmanaged/NFaceFactory.h"
#include "../Services/ModelService.h"

using namespace Xbim::Geometry::Services;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class FaceFactory : XbimHandle<NFaceFactory>, IXFaceFactory
			{
			private:
				
				ModelService^ _modelService;
				
			internal:
				TopoDS_Face BuildProfileDef(IIfcProfileDef^ profileDef);
				gp_Vec Normal(const TopoDS_Face& face);
			public:
				FaceFactory(ModelService^ modelService ) : XbimHandle(new NFaceFactory())
				{
					
					_modelService = modelService;;
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(_modelService->LoggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				TopoDS_Face BuildPlanarFace(IXPlane^ planeDef);
				
				virtual IXFace^ BuildFace(IXSurface^ surface, array<IXWire^>^ wires);
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _modelService->LoggingService; }};
			};
		}
	}
}

