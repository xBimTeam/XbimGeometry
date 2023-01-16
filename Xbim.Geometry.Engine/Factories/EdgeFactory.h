#pragma once

#include "Unmanaged/NEdgeFactory.h"
#include "FactoryBase.h"

#include <TopTools_DataMapOfIntegerShape.hxx>
#include <gp_Ax22d.hxx>
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class EdgeFactory : FactoryBase<NEdgeFactory>, IXEdgeFactory
			{


#pragma region IX..... interface methods. exposed publicly		

			public:
				
				virtual IXEdge^ Build(IXPoint^ start, IXPoint^ end);
				virtual IXEdge^ Build(IIfcCurve^ curve);
				virtual IXEdge^ Build(IXCurve^ curve);
				
#pragma endregion


#pragma region  Native Methods 

			protected:
				
			internal:
				
				TopoDS_Edge BuildEdge(IIfcCurve^ curve);

				TopoDS_Edge BuildEdge(Handle(Geom2d_Curve) hCurve2d);

				TopoDS_Edge BuildEdge(Handle(Geom_Curve) hCurve3d);

				TopoDS_Edge BuildEdgeCurve(IIfcEdgeCurve^ ifcEdgeCurve, TopTools_DataMapOfIntegerShape& verticesContext);
				TopoDS_Edge BuildCircle(double radius, gp_Ax22d axis);
#pragma endregion

#pragma region Constructors and private and protected data

			private:

			public:
				EdgeFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NEdgeFactory()) {}
#pragma endregion		
			};
		}
	}
}

