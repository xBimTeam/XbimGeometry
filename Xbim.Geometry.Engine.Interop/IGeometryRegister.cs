using Xbim.Common;
using Xbim.Geometry.Abstractions;

namespace Xbim.Geometry.Engine.Interop
{
    public interface IGeometryRegistration
    {
        void RegisterModel(IModel model);

        void UnregisterModel(IModel model);
    }
}
