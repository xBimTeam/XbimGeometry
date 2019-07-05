using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
    public interface IXbimScene
    {
        void Close();
        TransformGraph Graph { get; }
        // ReSharper disable once InconsistentNaming
        XbimLOD LOD { get; set; }
    }
}
