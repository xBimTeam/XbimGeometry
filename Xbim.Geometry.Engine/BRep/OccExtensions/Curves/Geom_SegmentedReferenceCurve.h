#pragma once

#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <TopLoc_Location.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <vector>
#include "./Geom_ConvertibleToBSpline.h"
#include "./Geom_GradientCurve.h"
#include "./Segments/Geom2d_Spiral.h"
#include "./Segments/Geom2d_Polynomial.h"


class Geom_SegmentedReferenceCurve;
DEFINE_STANDARD_HANDLE(Geom_SegmentedReferenceCurve, Geom_ConvertibleToBSpline)


class Geom_SegmentedReferenceCurve : public Geom_ConvertibleToBSpline {
public:

    Geom_SegmentedReferenceCurve(
        const Handle(Geom_GradientCurve)& baseCurve,
        const std::vector<std::pair<Handle(Geom2d_Curve), TopLoc_Location>>& superElevationFunction,
        const TopLoc_Location& endPoint,
        const bool hasEndPoint)
        : _baseCurve(baseCurve),
        _superElevationFunction(superElevationFunction),
        _endPoint(endPoint),
        _hasEndPoint(hasEndPoint)
    {
        _firstParam = _baseCurve->FirstParameter();
        _lastParam = _baseCurve->LastParameter();


        for (const auto& segmentPair : _superElevationFunction) {
            Handle(Geom2d_Curve) curve = segmentPair.first;
            Geom2dAdaptor_Curve adaptorCurve(curve);

            double firstParam = adaptorCurve.FirstParameter();
            double lastParam = adaptorCurve.LastParameter();

            double segmentLength = GCPnts_AbscissaPoint::Length(adaptorCurve, firstParam, lastParam);

            _totalLength += segmentLength;
            _cumulativeSuperelevationSpans.push_back(_totalLength);
        }
    }

    void D0(Standard_Real U, gp_Pnt& P) const
    {
        gp_Pnt basePnt;
        _baseCurve->D0(U, basePnt);

        std::tuple<Standard_Real, Standard_Real> superElevationAndTilt = GetSuperelevationAndCantTiltAt(U);

        P.SetCoord(basePnt.X(), basePnt.Y(), basePnt.Z() + std::get<0>(superElevationAndTilt));
    }

    void D1(Standard_Real U, gp_Pnt& P, gp_Vec& V) const
    {

        if (U < _firstParam) U = _firstParam;
        if (U > _lastParam) U = _lastParam;

        gp_Pnt basePnt;
        gp_Vec baseVec;
        _baseCurve->D1(U, basePnt, baseVec);
       
        std::tuple<Standard_Real, Standard_Real> superElevationAndTilt = GetSuperelevationAndCantTiltAt(U);

        P.SetCoord(basePnt.X(), basePnt.Y(), basePnt.Z() + std::get<0>(superElevationAndTilt));

        V = baseVec; 
    }

    void D2(Standard_Real U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2) const
    {
        if (U < _firstParam) U = _firstParam;
        if (U > _lastParam) U = _lastParam;

        gp_Pnt basePnt;
        gp_Vec baseVec1, baseVec2;
        _baseCurve->D2(U, basePnt, baseVec1, baseVec2);

        std::tuple<Standard_Real, Standard_Real> superElevationAndTilt = GetSuperelevationAndCantTiltAt(U);

        P.SetCoord(basePnt.X(), basePnt.Y(), basePnt.Z() + std::get<0>(superElevationAndTilt));

        V1 = baseVec1;
        V2 = baseVec2;
    }

    void D3(Standard_Real U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2, gp_Vec& V3) const
    {
        if (U < _firstParam) U = _firstParam;
        if (U > _lastParam) U = _lastParam;

        gp_Pnt basePnt;
        gp_Vec baseVec1, baseVec2, baseVec3;
        _baseCurve->D3(U, basePnt, baseVec1, baseVec2, baseVec3);

        std::tuple<Standard_Real, Standard_Real> superElevationAndTilt = GetSuperelevationAndCantTiltAt(U);

        P.SetCoord(basePnt.X(), basePnt.Y(), basePnt.Z() + std::get<0>(superElevationAndTilt));

        V1 = baseVec1;
        V2 = baseVec2;
        V3 = baseVec3;
    }

    gp_Vec DN(Standard_Real U, Standard_Integer N) const
    {
        if (N == 1) {
            gp_Pnt P;
            gp_Vec V1;
            D1(U, P, V1);
            return V1;
        }
        else if (N == 2) {
            gp_Pnt P;
            gp_Vec V1, V2;
            D2(U, P, V1, V2);
            return V2;
        }
        else if (N == 3) {
            gp_Pnt P;
            gp_Vec V1, V2, V3;
            D3(U, P, V1, V2, V3);
            return V2;
        }
        else {
            Standard_NotImplemented::Raise("DN not implemented for N > 3");
            return gp_Vec();
        }
    }

    Standard_Real FirstParameter() const
    {
        return _firstParam;
    }

    Standard_Real LastParameter() const
    {
        return _lastParam;
    }

    GeomAbs_Shape Continuity() const
    {
        return _baseCurve->Continuity();
    }

    Standard_Boolean IsClosed() const
    {
        gp_Pnt PStart, PEnd;
        D0(FirstParameter(), PStart);
        D0(LastParameter(), PEnd);

        return PStart.IsEqual(PEnd, Precision::Confusion());
    }

    Standard_Boolean IsPeriodic() const
    {
        return _baseCurve->IsPeriodic();
    }

    Standard_Boolean IsCN(Standard_Integer N) const {
        return _baseCurve->IsCN(N);
    }

    Standard_Real Geom_SegmentedReferenceCurve::ReversedParameter(Standard_Real U) const
    {
        return _firstParam + _lastParam - U;
    }

    void Geom_SegmentedReferenceCurve::Reverse()
    {
        Standard_NotImplemented::Raise("Reverse not implemented");

    }

    void Transform(const gp_Trsf& T)
    {
        Standard_NotImplemented::Raise("Transform not implemented");
    }

    Handle(Geom_Geometry) Copy() const
    {
        Handle(Geom_GradientCurve) baseCurveCopy = Handle(Geom_GradientCurve)::DownCast(_baseCurve->Copy());

        std::vector<std::pair<Handle(Geom2d_Curve), TopLoc_Location>> superElevationFunctionCopy;
        superElevationFunctionCopy.reserve(_superElevationFunction.size());

        for (const auto& segmentPair : _superElevationFunction) {
            Handle(Geom2d_Curve) curveCopy = Handle(Geom2d_Curve)::DownCast(segmentPair.first->Copy());
            TopLoc_Location locationCopy = segmentPair.second;
            superElevationFunctionCopy.emplace_back(curveCopy, locationCopy);
        }

        TopLoc_Location endPointCopy = _endPoint;

        Handle(Geom_SegmentedReferenceCurve) copy = new Geom_SegmentedReferenceCurve(
            baseCurveCopy,
            superElevationFunctionCopy,
            endPointCopy,
            _hasEndPoint
        );

        copy->_firstParam = _firstParam;
        copy->_lastParam = _lastParam;
        copy->_cumulativeSuperelevationSpans = _cumulativeSuperelevationSpans;
        copy->_totalLength = _totalLength;

        return copy;
    }


    Handle(Geom_BSplineCurve) ToBSpline(int nbPoints) const override
    {
        double baseCurveParm1 = _baseCurve->HorizontalCurve()->FirstParameter();
        double baseCurveParm2 = _baseCurve->HorizontalCurve()->LastParameter();

        TColgp_Array1OfPnt points(1, nbPoints + (_hasEndPoint ? 1 : 0));
        Geom2dAdaptor_Curve adaptorHorizontalProjection(_baseCurve->HorizontalCurve() , baseCurveParm1, baseCurveParm2);
        GCPnts_UniformAbscissa uniformAbscissa(adaptorHorizontalProjection, nbPoints);

        for (Standard_Integer i = 1; i <= nbPoints; ++i) {

            Standard_Real param = uniformAbscissa.Parameter(i);
            gp_Pnt2d basePoint;
            _baseCurve->HorizontalCurve()->D0(param, basePoint);

            std::tuple<Standard_Real, Standard_Real> superElevationAndTilt = GetSuperelevationAndCantTiltAt(param);

            Standard_Real superElevation = std::get<0>(superElevationAndTilt);
            
            gp_Pnt2d heightPoint;
            _baseCurve->HeightFunction()->D0(param, heightPoint);

            // Coord Z is the base gradient height + the the additional superelevation
            Standard_Real z = heightPoint.Y() + superElevation;
            gp_Pnt p3d(basePoint.X(), basePoint.Y(), z);
            points.SetValue(i, p3d);
        }

        if (_hasEndPoint) {
            //points.SetValue(nbPoints + 1, _endPoint.Transformation().TranslationPart());
        }

        GeomAPI_PointsToBSpline pointsToBSpline(points, 8, 8, GeomAbs_CN);
        return pointsToBSpline.Curve();
    }

    const Handle(Geom_GradientCurve)& BaseCurve() {
        return _baseCurve;
    }


    std::tuple<Standard_Real, Standard_Real> GetSuperelevationAndCantTiltAt(Standard_Real x_value) const
    {
        if (_superElevationFunction.empty()) {
            throw Standard_Failure("The curve segments vector is empty.");
        }

        if (x_value <= 0.0) {
            return std::make_tuple(_superElevationFunction.front().second.Transformation().TranslationPart().Y(), 0);
        }
        if (x_value >= _totalLength) {
            return std::make_tuple(_superElevationFunction.back().second.Transformation().TranslationPart().Y(), 0);
        }

        size_t segmentIndex = 0;
        for (; segmentIndex < _cumulativeSuperelevationSpans.size(); ++segmentIndex) {
            if (x_value <= _cumulativeSuperelevationSpans[segmentIndex]) {
                break;
            }
        }

        auto currentLoc = _superElevationFunction[segmentIndex].second;
        auto currentRateOfChange = _superElevationFunction[segmentIndex].first;
        Handle(Geom2d_Spiral) spiral = Handle(Geom2d_Spiral)::DownCast(currentRateOfChange);
        Handle(Geom2d_Polynomial) polynomial = Handle(Geom2d_Polynomial)::DownCast(currentRateOfChange);

        Standard_Real startTilt = GetRotationAroundX(currentLoc);
        Standard_Real delta1 = 0;
        Standard_Real delta2 = 0;
        Standard_Real deltaCurrent = spiral ? spiral->GetCurvatureAt(x_value - spiral->Placement().Location().X()) : 0;

        if (spiral) {
            delta1 = spiral->GetCurvatureAt(spiral->FirstParameter());
            delta2 = spiral->GetCurvatureAt(spiral->LastParameter());
            Standard_Real deltaCurrent = spiral->GetCurvatureAt(x_value - spiral->Placement().Location().X());
        }
        else if (polynomial)
        {
            delta1 = polynomial->GetCurvatureAt(polynomial->FirstParameter());
            delta2 = polynomial->GetCurvatureAt(polynomial->LastParameter());
            Standard_Real deltaCurrent = polynomial->GetCurvatureAt(x_value - polynomial->Placement().Location().X());
        }

        Standard_Real startSuperElevation = currentLoc.Transformation().TranslationPart().Y();

        const Standard_Real epsilon = 1e-9;

        // the superelevation rate of change is directly proportionate 
        // to the curve segment parent curve curvature gradient equation
        if (segmentIndex < _superElevationFunction.size() - 1)
        {
            // there is a next segment
            auto nextLoc = _superElevationFunction[segmentIndex + 1].second;
            Standard_Real endTilt = GetRotationAroundX(nextLoc);
            Standard_Real endSuperElevation = nextLoc.Transformation().TranslationPart().Y();

            Standard_Real denominator = delta2 - delta1;
            Standard_Real currentTilt;
            Standard_Real currentSuperElevation;

            if (spiral.IsNull() || polynomial.IsNull() || fabs(denominator) < epsilon) {
                currentTilt = startTilt;
                currentSuperElevation = startSuperElevation;
            }
            else {
                currentTilt = ((endTilt - startTilt) * (deltaCurrent - delta1) / denominator) + startTilt;
                currentSuperElevation = ((endSuperElevation - startSuperElevation) * (deltaCurrent - delta1) / denominator) + startSuperElevation;
            }

            return std::make_tuple(currentSuperElevation, currentTilt);
        }
        else {
            // no next segment
            auto endTilt = 0;
            auto endSuperElevation = 0;

            Standard_Real denominator = delta2 - delta1;
            Standard_Real currentTilt;
            Standard_Real currentSuperElevation;

            if (spiral.IsNull() || polynomial.IsNull() || fabs(denominator) < epsilon) {
                currentTilt = startTilt;
                currentSuperElevation = startSuperElevation;
            }
            else {
                currentTilt = ((endTilt - startTilt) * (deltaCurrent - delta1) / denominator) + startTilt;
                currentSuperElevation = ((endSuperElevation - startSuperElevation) * (deltaCurrent - delta1) / denominator) + startSuperElevation;
            }
            return std::make_tuple(currentSuperElevation, currentTilt);
        }
    }

private:
    Handle(Geom_GradientCurve) _baseCurve;
    std::vector<std::pair<Handle(Geom2d_Curve), TopLoc_Location>> _superElevationFunction;
    Standard_Real _firstParam;
    Standard_Real _lastParam;
    TopLoc_Location _endPoint;
    bool _hasEndPoint;
    std::vector<double> _cumulativeSuperelevationSpans;
    double _totalLength;

   
    Standard_Real GetRotationAroundX(TopLoc_Location loc) const{
        gp_Trsf trsf = loc.Transformation();
        // Rotation part:
        // | m11  m12  m13 |
        // | m21 [m22] m23 |
        // | m31 [m32] m33 |
        return atan2(trsf.Value(3, 2), trsf.Value(2, 2));;
    }
};

