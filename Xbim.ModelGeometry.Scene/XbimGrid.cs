using System.Collections.Generic;
using System.Linq;
using Xbim.Common.Geometry;
using Xbim.Common.XbimExtensions;
using Xbim.Ifc4.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimGrid
    {
        readonly Dictionary<IIfcGridAxis, IXbimCurve> _axis = new Dictionary<IIfcGridAxis, IXbimCurve>();
        readonly IIfcGrid _ifcGrid;
        public XbimGrid(IIfcGrid ifcGrid, IXbimGeometryEngine engine)
        {
            _ifcGrid = ifcGrid;
            foreach (var gridAxis in ifcGrid.UAxes)
            {
                IXbimCurve curve = engine.CreateCurve(gridAxis.AxisCurve);
                _axis.Add(gridAxis, curve);
            }
            foreach (var gridAxis in ifcGrid.VAxes)
            {
                IXbimCurve curve = engine.CreateCurve(gridAxis.AxisCurve);
                _axis.Add(gridAxis, curve);

            }
            foreach (var gridAxis in ifcGrid.WAxes)
            {
                IXbimCurve curve = engine.CreateCurve(gridAxis.AxisCurve);
                _axis.Add(gridAxis, curve);
            }

        }


        public XbimPoint3D? Position(IIfcVirtualGridIntersection intersect)
        {
            var axes = intersect.IntersectingAxes as IIfcGridAxis[] ?? intersect.IntersectingAxes.ToArray();
            if (axes.Count() != 2) return null;
            var ax1 = axes[0];
            var ax2 = axes[1];

            IXbimCurve curve1;
            IXbimCurve curve2;
            double tolerance = intersect.Model.ModelFactors.Precision;
            if (_axis.TryGetValue(ax1, out curve1) && _axis.TryGetValue(ax2, out curve2))
            {
                var hits = curve1.Intersections(curve2, tolerance);
                var xbimPoint3Ds = hits as XbimPoint3D[] ?? hits.ToArray();
                if (xbimPoint3Ds.Any())
                {
                    var pos = xbimPoint3Ds.FirstOrDefault();
                    var offset = intersect.OffsetDistances.AsTriplet();
                    pos = pos + new XbimVector3D(offset.A, offset.B, offset.C);
                    return pos;
                }
            }
            return null;
        }
    }
}