using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;
using Xunit;
namespace Xbim.Geometry.Engine.Interop.Tests
{
  
    public class GithubIssuesTests

    {

        ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());

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
                var c = new Xbim3DModelContext(m, loggerFactory, engineVersion);
                c.CreateContext(null, false);

                // todo: 2021: add checks so that the expected openings are correctly computed.
            }
        }

        
    }
}
