﻿using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.Diagnostics;
using Xbim.Common.Model;
using Xbim.Geometry.Abstractions;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;
using Xunit;
namespace Xbim.Geometry.Engine.Tests
{

    public class GithubIssuesTests

    {
        private readonly ILoggerFactory _loggerFactory;

        public GithubIssuesTests(ILoggerFactory loggerFactory)
        {
            _loggerFactory = loggerFactory;
        }
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Github_Issue_281(XGeometryEngineVersion engineVersion)
        {
            // this file resulted in a stack-overflow exception due to precision issues in the data.
            // We have added better exception management so that the stack-overflow is not thrown any more, 
            // however the voids in the wall are still not computed correctly.
            //
            using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
            {
                m.LoadStep21("TestFiles\\Github\\Github_issue_281_minimal.ifc");
                
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                var result = c.CreateContext(null, false);

                result.Should().Be(true);
            }
        }


        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Github_Issue473(XGeometryEngineVersion engineVersion)
        {
            // Performance of v6 engine very slow for complex BREPs, but much faster using the xbim Tesselator and skipping OCC.
            using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
            {
                m.LoadStep21(@"TestFiles\Github\Github_issue_473_minimal.ifc");
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                var timer = new Stopwatch();
                timer.Start();
                var result = c.CreateContext(null, false, false);
                timer.Stop();

                result.Should().Be(true);
                c.ShapeInstances().Should().HaveCount(4);
                c.ShapeGeometries().Should().HaveCount(4);

                timer.ElapsedMilliseconds.Should().BeLessThan(10000);
            }
        }



        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Cutting_Issue(XGeometryEngineVersion engineVersion)
        {

            using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
            {
                m.LoadStep21("TestFiles\\Github\\Dormitory-ARC_Opening_444.ifc");
                var c = new Xbim3DModelContext(m, _loggerFactory, engineVersion);
                c.CreateContext(null, false);

                var store = m.GeometryStore as InMemoryGeometryStore;

                var geom = store.ShapeGeometries.Values.First(c => c.IfcShapeLabel == 13519);

                geom.FaceCount.Should().Be(58);
                geom.Length.Should().Be(2221);

            }
        }


    }
}
