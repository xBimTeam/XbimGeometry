using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Web;
using Xbim.Common.Model;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;
using Xbim.Tessellator.MeshSimplification;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{
    public class MeshSimplificationTests
    {
        private readonly ILoggerFactory _loggerFactory;
        private ILogger _logger;

        public MeshSimplificationTests(ILoggerFactory loggerFactory)
        {
            _logger = loggerFactory.CreateLogger<MeshSimplificationTests>();
            _loggerFactory = loggerFactory;
        }


        [Theory]
        [InlineData(@"TestFiles\MeshSimplificationTestCases\IfcFlowTerminal.ifc")]
        public void CanSimplifyMesh(string file)
        {
            var reductionFactor = 0.3f;
            using var m = MemoryModel.OpenRead(file);
            var c = new Xbim3DModelContext(m, _loggerFactory, XGeometryEngineVersion.V6);
            c.MaxThreads = 1;
            var name = Path.GetFileName(file);
            var result = new SimplificationResult();
            result.FileName = name;

            var resultUnsimplified = c.CreateContext(null, false, postTessellationCallback: (mesh, id) =>
            {
                result.TrianglesBefore += mesh.TriangleCount;
                return mesh;
            });

            var nonSimplifiedFile = Path.ChangeExtension(file, ".original.wexbim");
            var st = File.Create(nonSimplifiedFile);
            using (var bw = new BinaryWriter(st))
            {
                m.SaveAsWexBim(bw);
                result.SizeBefore = bw.BaseStream.Length;
                bw.Close();
            }
            File.Delete(nonSimplifiedFile);

            (m.GeometryStore as InMemoryGeometryStore).ShapeGeometries.Clear();

            var resultSimplified = c.CreateContext(null, false, postTessellationCallback: (mesh, id) =>
            {
                var target = (int)Math.Max(mesh.TriangleCount * reductionFactor, 100);
                var simplifier = new XbimMeshSimplifier(mesh, (float)m.ModelFactors.Precision);
                mesh = simplifier.Simplify(target);
                result.TrianglesAfter += mesh.TriangleCount;
                return mesh;
            });

            var simplifiedFile = Path.ChangeExtension(file, ".simplified.wexbim");
            st = File.Create(simplifiedFile);
            using (var bw = new BinaryWriter(st))
            {
                m.SaveAsWexBim(bw);
                result.SizeAfter = bw.BaseStream.Length;
                bw.Close();
            }
            File.Delete(simplifiedFile);


            result.SizeAfter.Should().BeLessThan(result.SizeBefore / 3,
                  $"File size should be reduced for {result.FileName}");
            result.TrianglesAfter.Should().BeLessThan(result.TrianglesBefore / 3,
                $"Triangle count should be reduced for {result.FileName}");
        }

    }

    class SimplificationResult
    {
        public long SizeBefore { get; set; }
        public long SizeAfter { get; set; }
        public uint TrianglesBefore { get; set; }
        public uint TrianglesAfter { get; set; }
        public string FileName { get; set; }
    }
}
