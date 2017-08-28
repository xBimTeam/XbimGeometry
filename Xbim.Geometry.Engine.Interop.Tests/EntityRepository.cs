using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    public static class EntityRepository
    {
        public static IIfcGeometricRepresentationItem GetGeometry(string name)
        {
            using (var mm = MemoryModel.OpenRead($@"TestFiles\{name}.ifc"))
            {
                return mm.Instances[1] as IIfcGeometricRepresentationItem;
            }
        }
        public static T GetGeometry<T>(string name)
        {
            return (T)GetGeometry(name);
        }

    }
}
