#pragma once

using namespace Xbim::Common;
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			public ref class ModelService : IXModelService
			{
			private:
				IModel^ model;
				double minimumGap = 1; //1 mmm
			public:
				ModelService(IModel^ model, double minGapSize) {}
				virtual property double Precision {double get() { return model->ModelFactors->Precision; }};
				virtual property double OneMeter {double get() { return model->ModelFactors->OneMeter; }};
				virtual property double MinimumGap {double get() { return minimumGap; } void set(double distance) { minimumGap = distance; }};

			};
		}
	}
}
