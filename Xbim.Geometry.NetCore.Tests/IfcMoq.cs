using Moq;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using Xbim.Common;
using Xbim.Common.Metadata;
using Xbim.Ifc4;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.MeasureResource;

namespace Xbim.Geometry.NetCore.Tests
{
    public class IfcMoq
    {
        private static ExpressMetaData metaData = ExpressMetaData.GetMetadata(new EntityFactoryIfc4().GetType().GetTypeInfo().Module);
        #region Geometric processor Mocks
        public static IModel IfcModelMock(double millimetre = 1, double precision = 1e-5, double radianFactor = 1)
        {
            var modelMoq = new Mock<IModel>() { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            var model = modelMoq.Object;
            modelMoq.SetupGet(m => m.ModelFactors.Precision).Returns(precision);
            modelMoq.SetupGet(m => m.ModelFactors.OneMilliMeter).Returns(millimetre);
            modelMoq.SetupGet(m => m.ModelFactors.OneMeter).Returns(millimetre * 1000);
            modelMoq.SetupGet(m => m.ModelFactors.AngleToRadiansConversionFactor).Returns(radianFactor);
            return model;
        }
        public static IIfcVector IfcVector2dMock(double x = 0, double y = 1, double magnitude = 10)
        {
            var vecMoq = new Mock<IIfcVector>() { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            vecMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var vec = vecMoq.Object;
            vec.Orientation = IfcDirection2dMock(x, y);
            vec.Magnitude = magnitude;
            return vec;
        }
        public static IIfcVector IfcVector3dMock(double x = 0, double y = 0, double z = 1, double magnitude = 10)
        {
            var vecMoq = new Mock<IIfcVector>() { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            vecMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var vec = vecMoq.Object;
            vec.Orientation = IfcDirection3dMock(x, y, z);
            vec.Magnitude = magnitude;

            return vec;
        }
        public static IIfcDirection IfcDirection2dMock(double x = 0, double y = 1)
        {
            var dirMoq = new Mock<IIfcDirection>() { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            dirMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var dir = dirMoq.Object;
            dir.DirectionRatios.AddRange(new IfcReal[] { x, y });
            dirMoq.SetupGet(v => v.X).Returns(dir.DirectionRatios[0]);
            dirMoq.SetupGet(v => v.Y).Returns(dir.DirectionRatios[1]);
            return dir;
        }
        public static IIfcDirection IfcDirection3dMock(double x = 0, double y = 0, double z = 1)
        {
            var dirMoq = new Mock<IIfcDirection>() { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            dirMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var dir = dirMoq.Object;
            dir.DirectionRatios.AddRange(new IfcReal[] { x, y, z });
            dirMoq.SetupGet(v => v.X).Returns(dir.DirectionRatios[0]);
            dirMoq.SetupGet(v => v.Y).Returns(dir.DirectionRatios[1]);
            dirMoq.SetupGet(v => v.Z).Returns(dir.DirectionRatios[2]);
            return dir;
        }
        public static IIfcCartesianPoint IfcCartesianPoint2dMock(double x = 10, double y = 20)
        {
            var cpMoq = new Mock<IIfcCartesianPoint>() { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            cpMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var cp = cpMoq.Object;
            cp.Coordinates.AddRange(new IfcLengthMeasure[] { x, y });
            cpMoq.SetupGet(v => v.X).Returns(cp.Coordinates[0]);
            cpMoq.SetupGet(v => v.Y).Returns(cp.Coordinates[1]);
            return cp;
        }
        public static IIfcCartesianPoint IfcCartesianPoint3dMock(double x = 10, double y = 20, double z = 30)
        {
            var cpMoq = new Mock<IIfcCartesianPoint>()
            { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            cpMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var cp = cpMoq.Object;
            cp.Coordinates.AddRange(new IfcLengthMeasure[] { x, y, z });
            cpMoq.SetupGet(v => v.X).Returns(cp.Coordinates[0]);
            cpMoq.SetupGet(v => v.Y).Returns(cp.Coordinates[1]);
            cpMoq.SetupGet(v => v.Z).Returns(cp.Coordinates[2]);
            return cp;
        }

        public static IIfcAxis2Placement3D IfcIfcAxis2Placement3DMock(IIfcDirection axis = null, IIfcDirection refDir = null, IIfcCartesianPoint loc = null)
        {
            var axis3dMoq = new Mock<IIfcAxis2Placement3D>()
            { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            var axis3d = axis3dMoq.Object;
            axis3d.Axis = axis;
            axis3d.RefDirection = refDir;
            axis3d.Location = loc ?? IfcCartesianPoint3dMock();
            return axis3d;
        }
            #endregion
            #region Line Mocks
            public static IIfcLine IfcLine2dMock(IIfcCartesianPoint origin = null, IIfcVector direction = null)
        {
            var lineMoq = new Mock<IIfcLine>()
            { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            lineMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var line = lineMoq.Object;
            line.Pnt = origin ?? IfcCartesianPoint2dMock();
            line.Dir = direction ?? IfcVector2dMock();
            lineMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcLine)));
            return line;
        }
        public static IIfcLine IfcLine3dMock(IIfcCartesianPoint origin = null, IIfcVector direction = null)
        {
            var lineMoq = new Mock<IIfcLine>()
            { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            lineMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var line = lineMoq.Object;
            line.Pnt = origin ?? IfcCartesianPoint3dMock();
            line.Dir = direction ?? IfcVector3dMock();
            lineMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcLine)));
            return line;
        }
        #endregion

        #region Circle Mocks
        public static IIfcCircle IfcCircle3dMock(double radius = 100)
        {
            var circleMoq = new Mock<IIfcCircle>()
            { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            circleMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var circle = circleMoq.Object;
            circle.Radius = radius;
            circle.Position = IfcIfcAxis2Placement3DMock();
            circleMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcCircle)));
            return circle;
        }

        #endregion
        #region trimmed curve mocks
        public static IIfcTrimmedCurve IfcTrimmedCurve3dMock(IIfcCurve basisCurve = null, double trimParam1 = 0, double trimParam2 = 1, bool sense = true)
        {
            
            var trimMoq = new Mock<IIfcTrimmedCurve>()
            { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            trimMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var trim = trimMoq.Object;
            if (basisCurve == null)
                basisCurve = IfcLine3dMock();
            trim.BasisCurve = basisCurve;
            trim.MasterRepresentation = IfcTrimmingPreference.PARAMETER;
            trim.SenseAgreement = sense;
            trim.Trim1.Add(new IfcParameterValue(trimParam1));
            trim.Trim2.Add(new IfcParameterValue(trimParam2));            
            trimMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcTrimmedCurve)));           
            return trim;
        }
        public static IIfcTrimmedCurve IfcTrimmedCurve2dMock(IIfcCurve basisCurve = null, double trimParam1 = 0, double trimParam2 = 1, bool sense = true)
        {

            var trimMoq = new Mock<IIfcTrimmedCurve>()
            { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            trimMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var trim = trimMoq.Object;
            if (basisCurve == null)
                basisCurve = IfcLine2dMock();
            trim.BasisCurve = basisCurve;
            trim.MasterRepresentation = IfcTrimmingPreference.PARAMETER;
            trim.SenseAgreement = sense;
            trim.Trim1.Add(new IfcParameterValue(trimParam1));
            trim.Trim2.Add(new IfcParameterValue(trimParam2));
            trimMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcTrimmedCurve)));
            return trim;
        }
        #endregion
    }
}
