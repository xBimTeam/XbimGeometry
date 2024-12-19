#pragma once

using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common;
using namespace Microsoft::Extensions::Logging;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class GeometryConverterFactory :IXGeometryConverterFactory
			{
			public:
				virtual IXbimGeometryEngine^ CreateGeometryEngineV5(IModel^ model, ILoggerFactory^ loggerFactory);

				virtual IXModelGeometryService^ CreateModelGeometryService(IModel^ model, ILoggerFactory^ loggerFactory);
				virtual IXGeometryEngineV6^ CreateGeometryEngineV6(IModel^ model, ILoggerFactory^ loggerFactory);
				virtual IXbimGeometryEngine^ CreateGeometryEngine(XGeometryEngineVersion version,  IModel^ model, ILoggerFactory^ loggerFactory);
				virtual IXModelGeometryService^ GetUnderlyingModelGeometryService(IXbimGeometryEngine^ engine);
			};
		}
	}
}
