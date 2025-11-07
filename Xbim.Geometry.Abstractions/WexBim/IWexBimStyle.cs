using System.IO;

namespace Xbim.Geometry.Abstractions.WexBim
{
    public interface IWexBimStyle
    {
        float Alpha { get; set; }
        float Blue { get; set; }
        float Green { get; set; }
        float Red { get; set; }
        int StyleId { get; set; }

        void WriteToStream(BinaryWriter writer);
    }
}
