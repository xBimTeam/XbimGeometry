using System;
using System.Collections.Generic;
using System.Linq;

public class DynamicDeflectionSettings
{
    public double BaselineSectionWidthMm { get; set; } = 20.0;
    public int MinimumPerimeterFacets { get; set; } = 3;
    public int MaximumPerimeterFacets { get; set; } = 1000;
    public double CriticalSlenderness { get; set; } = 5;

    /// decrease to unrelax the coarseness
    public double MaxLinearDeflectionRatio { get; set; } = 1.5;
    public double MaxAngularDeflectionRadians { get; set; } = 1.5 * Math.PI;

    /// <summary>
    /// Optional custom strategy for advanced control.
    /// If set, this will override the simple MinimumPerimeterFacets × sizeRatio calculation.
    /// </summary>
    public CustomDeflectionStrategy CustomStrategy { get; set; } = null;

    public static DynamicDeflectionSettings ForTargetFacetCount(
        int targetPerimeterFacets,
        double baselineSectionWidthMm,
        int maximumPerimeterFacets = 1000,
        double criticalSlenderness = 10)
    {
        if (targetPerimeterFacets < 3)
            throw new ArgumentException("Target perimeter facets must be at least 3", nameof(targetPerimeterFacets));

        if (baselineSectionWidthMm <= 0)
            throw new ArgumentException("Reference section width must be positive", nameof(baselineSectionWidthMm));

        if (maximumPerimeterFacets < targetPerimeterFacets)
            throw new ArgumentException("Maximum perimeter facets must be greater than or equal to target perimeter facets", nameof(maximumPerimeterFacets));

        if (criticalSlenderness <= 0)
            throw new ArgumentException("Critical slenderness must be positive", nameof(criticalSlenderness));

        return new DynamicDeflectionSettings
        {
            BaselineSectionWidthMm = baselineSectionWidthMm,
            MinimumPerimeterFacets = targetPerimeterFacets,
            CriticalSlenderness = criticalSlenderness,
            MaximumPerimeterFacets = maximumPerimeterFacets
        };
    }


    public static DynamicDeflectionSettings WithCustomStrategy(CustomDeflectionStrategy strategy)
    {
        return new DynamicDeflectionSettings
        {
            CustomStrategy = strategy,
        };
    }
}


