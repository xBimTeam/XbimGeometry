#pragma once
#include "FactoryBase.h"
#include "Unmanaged/NBIMAuthoringToolWorkArounds.h"

/// <summary>
/// There are a number of legacy errors in the various BIM authoring tools, this class handles work arounds often to correct bad ifc definitions
/// </summary>

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class BIMAuthoringToolWorkArounds : FactoryBase<NBIMAuthoringToolWorkArounds>
			{
			internal:
				BIMAuthoringToolWorkArounds(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NBIMAuthoringToolWorkArounds()) {}
				/// <summary>
				/// Fixes a bug in early Revit exporters which incorrectly set the centre of arcs in IfcArbitraryOpenProfileDefs
				/// </summary>
				/// <param name="sweptProfile"></param>
				/// <param name="fixedEdge"></param>
				/// <returns></returns>
				bool FixRevitIncorrectArcCentreSweptCurve(IIfcSurfaceOfLinearExtrusion^ surfaceOfLinearExtrusions, TopoDS_Edge& fixedEdge);
				bool FixRevitSweptSurfaceExtrusionInFeet(gp_Vec& vec);
				bool FixRevitIncorrectBsplineSweptCurve(IIfcSurfaceOfLinearExtrusion^ surfaceOfLinearExtrusions, TopoDS_Edge& fixedEdge);

			private:
				System::Nullable<bool> ApplyRevitIncorrectArcCentreSweptCurve;
				System::Nullable<bool> ApplyRevitIncorrectBsplineSweptCurve;
				System::Nullable<bool> ApplyRevitSweptSurfaceExtrusionInFeet;

				bool ShouldApplyRevitIncorrectArcCentreSweptCurve();
				bool ShouldApplyRevitIncorrectBsplineSweptCurve();
				bool ShouldApplyRevitSweptSurfaceExtrusionInFeet();

				void InitRevitWorkArounds();
			};
		}
	}
}

