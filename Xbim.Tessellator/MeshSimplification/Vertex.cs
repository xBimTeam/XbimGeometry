namespace Xbim.Tessellator.MeshSimplification
{
    internal class Vertex
    {
        public Vec3 Position { get; set; }
        public Quadric Q { get; set; }
        public bool IsValid { get; set; }
    }
}
