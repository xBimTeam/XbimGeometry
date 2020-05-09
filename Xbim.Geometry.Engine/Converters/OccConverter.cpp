#include "OccConverter.h"
#include "../Services//Native/LoggingService.h"


int OccConverter::ToTransform(double m3D[], gp_Trsf& trsf)
{
	try
	{
		trsf.SetValues(m3D[0], m3D[4], m3D[8], m3D[12],
			m3D[1], m3D[5], m3D[9], m3D[13],
			m3D[2], m3D[6], m3D[10], m3D[14]);

		return 0;
	}
	catch (const std::exception& e)
	{
		LoggingService::LogError(e.what());
		return -1;
	}
}





