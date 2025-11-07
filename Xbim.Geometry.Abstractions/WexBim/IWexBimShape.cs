using System.IO;

namespace Xbim.Geometry.Abstractions.WexBim
{
    /// <summary>
    /// WesBimShape is a set of one or more faces that has a style and belongs  to 
    /// </summary>
    public interface IWexBimShape
    {
        bool IsSingleInstance { get; }
        int ProductLabel { get; set; }
        short InstanceTypeId { get; set; }
        int InstanceLabel { get; set; }
        int StyleId { get; set; }
        byte[] Transformation { get; set; }
        void WriteToStream(BinaryWriter writer);
    }
}
