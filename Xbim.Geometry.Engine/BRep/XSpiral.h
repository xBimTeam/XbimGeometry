//#pragma once
//
//#include "../XbimHandle.h"
//#include <Geom2d_Curve.hxx>
//#include <GCPnts_AbscissaPoint.hxx>
//#include <GeomAdaptor_Curve.hxx>
//
//using namespace Xbim::Geometry::Abstractions;
//
//using namespace Xbim::Common::Geometry;
//namespace Xbim
//{
//	namespace Geometry
//	{
//		namespace BRep
//		{
//			public ref class XSpiral : XbimHandle<Handle(Geom2d_Curve)>, IXSpiral
//			{
//			private:
//				XCurveType _curveType = XCurveType::IfcCurve;
//			public:
//				XSpiral(Handle(Geom2d_Curve) curve, XCurveType curveType) : XbimHandle(new Handle(Geom2d_Curve)(curve))
//				{
//					_curveType = curveType;
//				}
//
//				static const Handle(Geom2d_Curve) GeomCurve(IXSpiral^ xCurve);
//				virtual property XCurveType CurveType {XCurveType get() { return _curveType; }; };
//
//				property bool Is3d {virtual bool get() { return true; }; }
//				virtual property double FirstParameter {double get()
//				{
//					return OccHandle()->FirstParameter();
//				}};
//
//				virtual property double LastParameter {double get()
//				{
//					return OccHandle()->LastParameter();
//				}};
//				virtual IXPoint^ GetPoint(double uParam);
//				virtual System::Nullable<double> GetCurvature(double uParam);
//				virtual IXPoint^ GetFirstDerivative(double uParam, [System::Runtime::InteropServices::Out] IXDirection^% direction);
//				virtual IXPoint^ GetSecondDerivative(double uParam, [System::Runtime::InteropServices::Out] IXDirection^% direction, [System::Runtime::InteropServices::Out] IXDirection^% normal);
//				virtual property double Length { double get() { return 0;  } }
//			};
//		}
//	}
//}
//
