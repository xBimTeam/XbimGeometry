using System.Collections.Generic;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.ModelGeometry.Scene
{
    public class XbimGridCollection:List<XbimGrid>
    {
        public XbimGridCollection(IEnumerable<IIfcGrid> ifcGrids, IXbimGeometryEngine engine)
        {
            foreach (var ifcGrid in ifcGrids)
            {
                Add(new XbimGrid(ifcGrid, engine));
            }
        }

        public XbimPoint3D? Position(IIfcVirtualGridIntersection intersect)
        {
            //check all the grids to see which one has the placement
            foreach (var grid in this)
            {
                var pos = grid.Position(intersect);
                if (pos.HasValue) return pos;
            }
            return null;
        }
    }
}
