using System;
using System.Collections.Generic;
using System.Linq;

public class DeflectionControlPoint
{
    public double SectionWidthMm { get; set; }
    public double SlendernessRatio { get; set; }
    public int TargetFacets { get; set; }

    public DeflectionControlPoint(double sectionWidthMm, double slendernessRatio, int targetFacets)
    {
        SectionWidthMm = sectionWidthMm;
        SlendernessRatio = slendernessRatio;
        TargetFacets = targetFacets;
    }

    public override string ToString()
    {
        return $"SectionWidth: {SectionWidthMm}mm, Slenderness: {SlendernessRatio}, TargetFacets: {TargetFacets}";
    }
}


public enum StrategyType
{
    BilinearInterpolation,
    NearestNeighbor,
}

public class CustomDeflectionStrategy
{
    private const double NumericalTolerance = 1e-10;
    private readonly StrategyType _strategyType;

    public CustomDeflectionStrategy(StrategyType strategyType = StrategyType.BilinearInterpolation)
    {
        _strategyType = strategyType;
    }

    public List<DeflectionControlPoint> ControlPoints { get; set; } = new();


    public int GetTargetFacets(double sectionWidthMm, double slendernessRatio)
    {
        if (!ControlPoints.Any())
            return 6;

        if (_strategyType == StrategyType.BilinearInterpolation)
        {
            var interpolated = BilinearInterpolate(sectionWidthMm, slendernessRatio);
            if (interpolated.HasValue)
                return Math.Max(3, (int)Math.Round(interpolated.Value)); // Minimum 3 facets
        }

        // If NearestNeighbor or bilinear interpolation fails
        var closest = ControlPoints
            .OrderBy(cp => Math.Pow(cp.SectionWidthMm - sectionWidthMm, 2) +
                          Math.Pow(cp.SlendernessRatio - slendernessRatio, 2))
            .FirstOrDefault();

        return closest?.TargetFacets ?? 6;
    }


    private double? BilinearInterpolate(double x, double y)
    {
        var xValues = ControlPoints.Select(cp => cp.SectionWidthMm).Distinct().OrderBy(v => v).ToArray();
        var yValues = ControlPoints.Select(cp => cp.SlendernessRatio).Distinct().OrderBy(v => v).ToArray();

        if (xValues.Length < 2 || yValues.Length < 2)
            return null;

        var x1 = xValues.LastOrDefault(xv => xv <= x);
        var x2 = xValues.FirstOrDefault(xv => xv >= x);

        var y1 = yValues.LastOrDefault(yv => yv <= y);
        var y2 = yValues.FirstOrDefault(yv => yv >= y);

        if (x1 == 0 && x2 == 0) // x is below all values
        {
            x1 = xValues[0];
            x2 = xValues.Length > 1 ? xValues[1] : x1;
        }
        else if (x1 == x2) // x is above all values or exactly on a point
        {
            var index = Array.IndexOf(xValues, x1);
            if (index > 0)
            {
                x1 = xValues[index - 1];
            }
            else if (index < xValues.Length - 1)
            {
                x2 = xValues[index + 1];
            }
        }

        if (y1 == 0 && y2 == 0) // y is below all values
        {
            y1 = yValues[0];
            y2 = yValues.Length > 1 ? yValues[1] : y1;
        }
        else if (y1 == y2) // y is above all values or exactly on a point
        {
            var index = Array.IndexOf(yValues, y1);
            if (index > 0)
            {
                y1 = yValues[index - 1];
            }
            else if (index < yValues.Length - 1)
            {
                y2 = yValues[index + 1];
            }
        }

        var f11 = GetValueAt(x1, y1);
        var f12 = GetValueAt(x1, y2);
        var f21 = GetValueAt(x2, y1);
        var f22 = GetValueAt(x2, y2);

        if (!f11.HasValue || !f12.HasValue || !f21.HasValue || !f22.HasValue)
            return null;

        if (Math.Abs(x2 - x1) < NumericalTolerance && Math.Abs(y2 - y1) < NumericalTolerance)
            return f11.Value;

        if (Math.Abs(x2 - x1) < NumericalTolerance)
            return LinearInterpolate(y, y1, y2, f11.Value, f12.Value); // Vertical line

        if (Math.Abs(y2 - y1) < NumericalTolerance)
            return LinearInterpolate(x, x1, x2, f11.Value, f21.Value); // Horizontal line

        var denom = (x2 - x1) * (y2 - y1);
        return (f11.Value * (x2 - x) * (y2 - y) +
                f21.Value * (x - x1) * (y2 - y) +
                f12.Value * (x2 - x) * (y - y1) +
                f22.Value * (x - x1) * (y - y1)) / denom;
    }

    private int? GetValueAt(double x, double y)
    {
        const double tolerance = 1e-6;
        return ControlPoints
            .FirstOrDefault(cp => Math.Abs(cp.SectionWidthMm - x) < tolerance &&
                                 Math.Abs(cp.SlendernessRatio - y) < tolerance)
            ?.TargetFacets;
    }

    private double LinearInterpolate(double x, double x1, double x2, double y1, double y2)
    {
        if (Math.Abs(x2 - x1) < NumericalTolerance)
            return y1;

        return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
    }


    public CustomDeflectionStrategy AddPoint(double sectionWidth, double slenderness, int facets)
    {
        ControlPoints.Add(new DeflectionControlPoint(sectionWidth, slenderness, facets));
        return this;
    }

}