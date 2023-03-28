#include "PackedNormal.h"
#include <math.h>



PackedNormal PackedNormal::ToPackedNormal(const gp_Dir& vec)
{
	const double PackTolerance = std::tan(1 / PackSize);
	static  PackedNormal SingularityNegativeY((unsigned char)PackSize, (unsigned char)PackSize);
	static  PackedNormal SingularityPositiveY(0, 0);
	static double HalfPI = M_PI / 2;
	static double PIplusHalfPI = M_PI + HalfPI;
	static double TwoPI = M_PI * 2;
	/*int length = vec.Length();
	if (std::Abs(length - 1.0) > 1e-5)
	{
		throw new Exception("Vector has to be normalized before it is packed.");
	}*/

	//the most basic case when normal points in Y direction (singular point)
	if (std::abs(1 - vec.Y()) < PackTolerance)
		return SingularityPositiveY;
	//the most basic case when normal points in -Y direction (second singular point)
	if (std::abs(vec.Y() + 1) < PackTolerance)
		return SingularityNegativeY;

	double lat; //effectively XY
	double lon; //effectively Z
	//special cases when vector is aligned to one of the axis
	if (std::abs(vec.Z() - 1) < PackTolerance)
	{
		lon = 0;
		lat = HalfPI;
	}
	else if (std::abs(vec.Z() + 1) < PackTolerance)
	{
		lon = M_PI;
		lat = HalfPI;
	}
	else if (std::abs(vec.X() - 1) < PackTolerance)
	{
		lon = HalfPI;
		lat = HalfPI;
	}
	else if (std::abs(vec.X() + 1) < PackTolerance)
	{
		lon = PIplusHalfPI;
		lat = HalfPI;
	}
	else
	{
		//Atan2 takes care for quadrants (goes from positive Z to positive X and around)
		lon = std::atan2(vec.X(), vec.Z());
		//latitude from 0 to PI starting at positive Y ending at negative Y
		lat = std::acos(vec.Y());
	}

	//normalize values
	lon = lon / TwoPI;
	lat = lat / M_PI;

	//stretch to pack size so that round directions are aligned to axes.
	PackedNormal result((unsigned char)(lon * PackSize), (unsigned char)(lat * PackSize));
	return result;
}

gp_Dir PackedNormal::ToNormal()
{
	auto lon = byte[0] / PackSize * M_PI * 2;
	auto lat = byte[1] / PackSize * M_PI;
	auto y = std::cos(lat);
	auto x = std::sin(lon) * std::sin(lat);
	auto z = std::cos(lon) * std::sin(lat);
	return gp_Dir(x, y, z);
}