#include "ModelService.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			ModelService::ModelService(IModel^ iModel, double minGapSize) 
				: minimumGap(minGapSize), 
				precisionSquared(model->ModelFactors->Precision * model->ModelFactors->Precision), 
				model(iModel)
			{
			
			}
			ModelService::ModelService(IModel^ iModel) 
				: ModelService(iModel, model->ModelFactors->OneMilliMeter)
			{
			
			}
		}
	}
}