namespace Xbim.Geometry.Abstractions
{
    public interface IXShapeColour : IXShapeItem
    {
        IXColourRGB AmbientColor { get; set; }
        IXColourRGB DiffuseColor { get; set; }
        IXColourRGB SpecularColor { get; set; }
        IXColourRGB EmissiveColor { get; set; }
        /// <summary>
        /// Value between 0.0 and 1.0
        /// </summary>
        float Shininess { get; set; }
        /// <summary>
        /// Value between 0.0 and 1.0
        /// </summary>
        float Transparency { get; set; }
    }
}
