#include "ModelService.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			ModelService::ModelService(IModel^ iModel, double minGapSize) 
				: minimumGap(minGapSize), model(iModel)
			{
			
			}
			ModelService::ModelService(IModel^ iModel) 
				: minimumGap(model->ModelFactors->OneMilliMeter), model(iModel)
			{
			
			}
		}
	}
}