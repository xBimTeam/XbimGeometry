using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Common.Model;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;
using Xunit;


namespace Xbim.Geometry.Engine.Tests
{

    public class DynamicDeflectionTests : IClassFixture<DynamicDeflectionTests>
    {
        private readonly ILoggerFactory _loggerFactory;
        private ILogger _logger;
        private readonly IXbimGeometryServicesFactory _factory;


        public DynamicDeflectionTests(ILoggerFactory loggerFactory, IXbimGeometryServicesFactory factory)
        {
            _logger = loggerFactory.CreateLogger<DynamicDeflectionTests>();
            _loggerFactory = loggerFactory;
            _factory = factory;
        }

        private class SimplificationResult
        {
            public long SizeBefore { get; set; }
            public long SizeAfter { get; set; }
        }

        [Theory]
        [InlineData("TestFiles\\DynamicDeflectionTestCases\\Pipe7260.ifc")]
        [InlineData("TestFiles\\DynamicDeflectionTestCases\\Pipe7310.ifc")]
        [InlineData("TestFiles\\DynamicDeflectionTestCases\\ReinforcingBar1.ifc")]
        [InlineData("TestFiles\\DynamicDeflectionTestCases\\ReinforcingBar2.ifc")]
        public void CanSimplifyShapesByApplyingDynamicDeflectionWithCustomStrategy(string filePath)
        {
            using var model = MemoryModel.OpenRead(filePath);
            var c = new Xbim3DModelContext(model, _loggerFactory, XGeometryEngineVersion.V6);
            var result = new SimplificationResult();

            var created = c.CreateContext(null, true);
            created.Should().BeTrue();

            var nonSimplifiedFile = Path.ChangeExtension(filePath, ".original.wexbim");
            var st = File.Create(nonSimplifiedFile);
            using (var bw = new BinaryWriter(st))
            {
                model.SaveAsWexBim(bw);
                result.SizeBefore = bw.BaseStream.Length;
                bw.Close();
            }
            File.Delete(nonSimplifiedFile);

            (model.GeometryStore as InMemoryGeometryStore).ShapeGeometries.Clear();

            // With dynamic deflection
            var c2 = new Xbim3DModelContext(model, _loggerFactory, XGeometryEngineVersion.V6);

            var strategy = new CustomDeflectionStrategy()
                .AddPoint(sectionWidth: 5, slenderness: 5, facets: 3)
                .AddPoint(sectionWidth: 10, slenderness: 5, facets: 3)
                .AddPoint(sectionWidth: 10, slenderness: 10, facets: 3)
                .AddPoint(sectionWidth: 20, slenderness: 10, facets: 3)
                .AddPoint(sectionWidth: 20, slenderness: 20, facets: 3);

            var settings = DynamicDeflectionSettings.WithCustomStrategy(strategy);

            created = c.CreateContext(null, true, dynamicDeflectionSettings: settings);
            created.Should().BeTrue();

            var simplifiedFile = Path.ChangeExtension(filePath, ".simplified.wexbim");
            st = File.Create(simplifiedFile);
            using (var bw = new BinaryWriter(st))
            {
                model.SaveAsWexBim(bw);
                result.SizeAfter = bw.BaseStream.Length;
                bw.Close();
            }
            File.Delete(simplifiedFile);

            // Assert
            result.SizeAfter.Should().BeLessThan(result.SizeBefore,
                $"File size should be reduced for {Path.GetFileName(filePath)}");
        }


        [Theory]
        [InlineData("TestFiles\\DynamicDeflectionTestCases\\Pipe7260.ifc")]
        [InlineData("TestFiles\\DynamicDeflectionTestCases\\Pipe7310.ifc")]
        [InlineData("TestFiles\\DynamicDeflectionTestCases\\ReinforcingBar1.ifc")]
        [InlineData("TestFiles\\DynamicDeflectionTestCases\\ReinforcingBar2.ifc")]
        public void CanSimplifyShapesByApplyingDynamicDeflectionForTargetFacetCount(string filePath)
        {
            using var model = MemoryModel.OpenRead(filePath);
            var c = new Xbim3DModelContext(model, _loggerFactory, XGeometryEngineVersion.V6);
            var result = new SimplificationResult();

            var created = c.CreateContext(null, true);
            created.Should().BeTrue();

            var nonSimplifiedFile = Path.ChangeExtension(filePath, ".original.wexbim");
            var st = File.Create(nonSimplifiedFile);
            using (var bw = new BinaryWriter(st))
            {
                model.SaveAsWexBim(bw);
                result.SizeBefore = bw.BaseStream.Length;
                bw.Close();
            }
            File.Delete(nonSimplifiedFile);

            (model.GeometryStore as InMemoryGeometryStore).ShapeGeometries.Clear();

            // With dynamic deflection
            var c2 = new Xbim3DModelContext(model, _loggerFactory, XGeometryEngineVersion.V6);

            var settings = DynamicDeflectionSettings.ForTargetFacetCount(3, 40);

            created = c.CreateContext(null, true, dynamicDeflectionSettings: settings);
            created.Should().BeTrue();

            var simplifiedFile = Path.ChangeExtension(filePath, ".simplified.wexbim");
            st = File.Create(simplifiedFile);
            using (var bw = new BinaryWriter(st))
            {
                model.SaveAsWexBim(bw);
                result.SizeAfter = bw.BaseStream.Length;
                bw.Close();
            }
            File.Delete(simplifiedFile);

            // Assert
            result.SizeAfter.Should().BeLessThan(result.SizeBefore,
                $"File size should be reduced for {Path.GetFileName(filePath)}");
        }
    }
}
