#pragma once
#include "../XbimHandle.h"
#include "./Unmanaged/NFaceFactory.h"
#include "../Services/LoggingService.h"
#include "GeometryProcedures.h"
#include "WireFactory.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class FaceFactory : XbimHandle<NFaceFactory>, IXFaceFactory
			{
			private:
				IXLoggingService^ _loggerService;
				IXModelService^ _modelService;
				GeometryProcedures^ GPFactory;
				WireFactory^ _wireFactory;
			internal:
				TopoDS_Face BuildProfileDef(IIfcProfileDef^ profileDef);
				gp_Vec Normal(const TopoDS_Face& face);
			public:
				FaceFactory(IXLoggingService^ loggingService, IXModelService^ modelService, IXWireFactory^ wireFactory ) : XbimHandle(new NFaceFactory())
				{
					_loggerService = loggingService;
					_modelService = modelService;
					GPFactory = gcnew GeometryProcedures(loggingService, modelService);
					_wireFactory = dynamic_cast<WireFactory^>(wireFactory);
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				TopoDS_Face BuildPlanarFace(IXPlane^ planeDef);
				
				virtual IXFace^ BuildFace(IXSurface^ surface, array<IXWire^>^ wires);
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _loggerService; }};
			};
		}
	}
}

