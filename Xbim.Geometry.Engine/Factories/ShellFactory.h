#pragma once
#include "FactoryBase.h"
#include <TopoDS_Shell.hxx>
#include "./Unmanaged/NShellFactory.h"
#include "../Services/LoggingService.h"
#include "CheckClosedStatus.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class ShellFactory : FactoryBase<NShellFactory>, IXShellFactory
			{
				
			public:
				ShellFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NShellFactory()){}
				TopoDS_Shape BuildClosedShell(IIfcClosedShell^ closedShell, bool& isFixed);
				TopoDS_Shape BuildConnectedFaceSet(IIfcConnectedFaceSet^ faceSet, bool& isFixed);
				TopoDS_Shape FixShell(TopoDS_Shell& shell, IPersistEntity^ entity, bool& isFixed);
				TopoDS_Shape BuildConnectedFaceSurfaceSet(IIfcConnectedFaceSet^ faceSet, bool& isFixed);
				TopoDS_Shape BuildPolygonalFaceSet(IIfcPolygonalFaceSet^ faceSet, bool& isFixed);
			};
		}
	}

};

