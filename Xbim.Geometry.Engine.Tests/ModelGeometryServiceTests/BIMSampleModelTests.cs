using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Common;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{
    /// <summary>
    /// This test class tests each of the letters in the BIM test sample model for correct conversion
    /// </summary>
    public class BIMSampleModelTests
    {
        #region Setup

        private readonly ILoggerFactory _loggerFactory;
        private readonly IXbimGeometryServicesFactory factory;
        readonly IXModelGeometryService _modelSvc;
        public BIMSampleModelTests(ILoggerFactory loggerFactory, IXbimGeometryServicesFactory factory)
        {
            _loggerFactory = loggerFactory;
            this.factory = factory;
            var bimSampleModel = MemoryModel.OpenRead("TestFiles/Github/BIM Logo.ifc");
            _modelSvc = factory.CreateModelGeometryService(bimSampleModel, _loggerFactory);
        }
        #endregion

        /// <summary>
        /// Letter B Test
        /// </summary>
        [Fact]
        public void Can_Convert_Letter_B()
        {
            var ifc = _modelSvc.Model.Instances[88] as IIfcCsgSolid;
            var occ = _modelSvc.SolidFactory.Build(ifc) as IXSolid;
            occ.Should().NotBeNull();
            occ.Shells.Count().Should().Be(1);
            occ.Shells[0].Faces.Count().Should().Be(15);
            occ.Volume.Should().BeApproximately(775084358, 1);
        }

        /// <summary>
        /// Exclaimation mark
        /// </summary>
        [Fact]
        public void Can_Convert_Exclaimation_Mark_Body()
        {
            var ifc = _modelSvc.Model.Instances[233] as IIfcFacetedBrep;
            var occ = _modelSvc.SolidFactory.Build(ifc) as IXSolid;
            occ.Should().NotBeNull();
            occ.Shells.Count().Should().Be(1);
            occ.Shells[0].Faces.Count().Should().Be(6);
            occ.Volume.Should().BeApproximately(228000000, 1);

        }

        [Fact]
        public void Can_Convert_Exclaimation_Mark_Dot()
        {
            var ifc = _modelSvc.Model.Instances[328] as IIfcRevolvedAreaSolid;
            var occ = _modelSvc.SolidFactory.Build(ifc) as IXSolid;
            occ.Should().NotBeNull();
            //var brep = occ.BrepString();
            occ.Shells.Count().Should().Be(1);
            occ.Shells[0].Faces.Count().Should().Be(1);
            occ.Volume.Should().BeApproximately(11494040, 1);
        }


        [Fact]
        public void Can_Convert_Letter_I_Dot()
        {
            var ifc = _modelSvc.Model.Instances[121] as IIfcRevolvedAreaSolid;
            var occ = _modelSvc.SolidFactory.Build(ifc) as IXSolid;
            occ.Should().NotBeNull();
            //var brep = occI.BrepString();
            occ.Shells.Count().Should().Be(1);
            occ.Shells[0].Faces.Count().Should().Be(4);
            occ.Volume.Should().BeApproximately(47123889, 1);
        }
        [Fact]
        public void Can_Convert_Letter_I_Body()
        {
            var ifc = _modelSvc.Model.Instances[107] as IIfcExtrudedAreaSolid;
            var occ = _modelSvc.SolidFactory.Build(ifc) as IXSolid;
            occ.Should().NotBeNull();
            var brep = occ.BrepString();
            occ.Shells.Count().Should().Be(1);
            occ.Shells[0].Faces.Count().Should().Be(6);
            occ.Volume.Should().BeApproximately(300000000, 1);
        }
        [Fact]
        public void Can_Convert_Ruled_Line()
        {
            var ifc = _modelSvc.Model.Instances[97] as IIfcExtrudedAreaSolid;
            var occ = _modelSvc.SolidFactory.Build(ifc) as IXSolid;
            occ.Should().NotBeNull();
            //var brep = occI.BrepString();
            occ.Shells.Count().Should().Be(1);
            occ.Shells[0].Faces.Count().Should().Be(6);
            occ.Volume.Should().BeApproximately(192000000, 1);
        }

        [Fact]
        public void Can_Convert_Letter_M()
        {
            var ifc = _modelSvc.Model.Instances[158] as IIfcSurfaceCurveSweptAreaSolid;
            var occ = _modelSvc.SolidFactory.Build(ifc) as IXSolid;
            occ.Should().NotBeNull();
            var brep = occ.BrepString();
            occ.Shells.Count().Should().Be(1);
            occ.Shells[0].Faces.Count().Should().Be(26);
            occ.Volume.Should().BeApproximately(870231920, 1);
        }
    }
}
