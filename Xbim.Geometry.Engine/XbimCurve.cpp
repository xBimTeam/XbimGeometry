#include "XbimCurve.h"


using namespace System;

namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimCurve::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (Handle_Geom_Curve*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

		XbimCurve::XbimCurve(const Handle_Geom_Curve& curve, double p1, double p2)
		{
			this->pCurve = new Handle_Geom_Curve();
			*pCurve = curve;
			startParam = p1;
			endParam = p2;
		}

		IXbimGeometryObject^ XbimCurve::Transform(XbimMatrix3D matrix3D)
		{
			throw gcnew Exception("Tranformation of curves is not currently supported");
		}
	}
}
