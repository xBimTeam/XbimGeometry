using Moq;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using Xbim.Common;
using Xbim.Common.Metadata;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc4;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.MeasureResource;

namespace Xbim.Geometry.NetCore.Tests
{
    public static partial class IfcMoq
    {
        private static Mock<T> MakeMoq<T>() where T : class, IPersistEntity
        {
            return new Mock<T>()
            {
                DefaultValue = DefaultValue.Mock,
                DefaultValueProvider = new MoqDefaultBehaviourProvider()
            }.SetupAllProperties();
        }

        private static ExpressMetaData metaData = ExpressMetaData.GetMetadata(new EntityFactoryIfc4().GetType().GetTypeInfo().Module);
        #region Geometric processor Mocks
        public static IModel IfcModelMock(double millimetre = 1, double precision = 1e-5, double radianFactor = Math.PI/180)
        {
            var modelMoq = new Mock<IModel>()
            {
                DefaultValue = DefaultValue.Mock,
                DefaultValueProvider = new MoqDefaultBehaviourProvider()
            }.SetupAllProperties();

            var model = modelMoq.Object;
            modelMoq.SetupGet(m => m.ModelFactors.Precision).Returns(precision);
            modelMoq.SetupGet(m => m.ModelFactors.OneMilliMeter).Returns(millimetre);
            modelMoq.SetupGet(m => m.ModelFactors.OneMilliMetre).Returns(millimetre);
            modelMoq.SetupGet(m => m.ModelFactors.OneMeter).Returns(millimetre * 1000);
            modelMoq.SetupGet(m => m.ModelFactors.AngleToRadiansConversionFactor).Returns(radianFactor);
            return model;
        }
        public static IIfcVector IfcVector2dMock(IIfcDirection direction = null, double magnitude = 10)
        {
            var vecMoq = MakeMoq<IIfcVector>();
            vecMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var vec = vecMoq.Object;
            vec.Orientation = direction ?? IfcDirection2dMock();
            vec.Magnitude = magnitude;
            return vec;
        }
        public static IIfcVector IfcVector3dMock(IIfcDirection direction = null, double magnitude = 10)
        {
            var vecMoq = MakeMoq<IIfcVector>();
            vecMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var vec = vecMoq.Object;
            vec.Orientation = direction ?? IfcDirection3dMock();
            vec.Magnitude = magnitude;

            return vec;
        }
        public static IIfcDirection IfcDirection2dMock(double x = 1, double y = 0)
        {
            var dirMoq = MakeMoq<IIfcDirection>();
            dirMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var dir = dirMoq.Object;
            dir.DirectionRatios.AddRange(new IfcReal[] { x, y });
            dirMoq.SetupGet(v => v.X).Returns(dir.DirectionRatios[0]);
            dirMoq.SetupGet(v => v.Y).Returns(dir.DirectionRatios[1]);
            return dir;
        }
        public static IIfcDirection IfcDirection3dMock(double x = 0, double y = 0, double z = 1)
        {
            var dirMoq = MakeMoq<IIfcDirection>();
            dirMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var dir = dirMoq.Object;
            dir.DirectionRatios.AddRange(new IfcReal[] { x, y, z });
            dirMoq.SetupGet(v => v.X).Returns(dir.DirectionRatios[0]);
            dirMoq.SetupGet(v => v.Y).Returns(dir.DirectionRatios[1]);
            dirMoq.SetupGet(v => v.Z).Returns(dir.DirectionRatios[2]);
            return dir;
        }
        public static IIfcCartesianPoint IfcCartesianPoint2dMock(double x = 0, double y = 0, int ifcLabel = 1)
        {
            var cpMoq = MakeMoq<IIfcCartesianPoint>();
            cpMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var cp = cpMoq.Object;
            cp.Coordinates.AddRange(new IfcLengthMeasure[] { x, y });
            cpMoq.SetupGet(v => v.X).Returns(cp.Coordinates[0]);
            cpMoq.SetupGet(v => v.Y).Returns(cp.Coordinates[1]);
            cpMoq.SetupGet(v => v.EntityLabel).Returns(ifcLabel);
            return cp;
        }
        public static IIfcCartesianPoint IfcCartesianPoint3dMock(double x = 0, double y = 0, double z = 0, int ifcLabel = 1)
        {
            var cpMoq = MakeMoq<IIfcCartesianPoint>();
            cpMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var cp = cpMoq.Object;
            cp.Coordinates.AddRange(new IfcLengthMeasure[] { x, y, z });
            cpMoq.SetupGet(v => v.X).Returns(cp.Coordinates[0]);
            cpMoq.SetupGet(v => v.Y).Returns(cp.Coordinates[1]);
            cpMoq.SetupGet(v => v.Z).Returns(cp.Coordinates[2]);
            cpMoq.SetupGet(v => v.EntityLabel).Returns(ifcLabel);
            return cp;
        }

        public static IIfcAxis2Placement3D IfcAxis2Placement3DMock(IIfcDirection axis = null, IIfcDirection refDir = null, IIfcCartesianPoint loc = null)
        {
            var axis3dMoq = MakeMoq<IIfcAxis2Placement3D>();
            var axis3d = axis3dMoq.Object;
            axis3d.Axis = axis??IfcDirection3dMock();
            axis3d.RefDirection = refDir;
            axis3d.Location = loc ?? IfcCartesianPoint3dMock();
            return axis3d;
        }

        public static IIfcAxis2Placement2D IfcAxis2Placement2DMock(IIfcDirection refDir = null, IIfcCartesianPoint loc = null)
        {
            var axis2dMoq = MakeMoq<IIfcAxis2Placement2D>();
            var axis2d = axis2dMoq.Object;
            axis2d.RefDirection = refDir;
            axis2d.Location = loc ?? IfcCartesianPoint2dMock();
            return axis2d;
        }

        #endregion

        #region Line Mocks
        public static IIfcLine IfcLine2dMock(IIfcCartesianPoint origin = null, double magnitude = 1, IIfcDirection direction = null)
        {
            var lineMoq = MakeMoq<IIfcLine>();
            lineMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var line = lineMoq.Object;
            line.Pnt = origin ?? IfcCartesianPoint2dMock();
            line.Dir = IfcVector2dMock(direction: direction, magnitude: magnitude);
            lineMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcLine)));
            return line;
        }
        public static IIfcLine IfcLine3dMock(IIfcCartesianPoint origin = null, double magnitude = 1, IIfcDirection direction = null)
        {
            var lineMoq = MakeMoq<IIfcLine>();
            lineMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var line = lineMoq.Object;
            line.Pnt = origin ?? IfcCartesianPoint3dMock();
            line.Dir = IfcVector3dMock(direction: direction, magnitude: magnitude);
            lineMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcLine)));
            return line;
        }
        #endregion

        #region Circle Mocks
        public static IIfcCircle IfcCircle3dMock(double radius = 100, IIfcAxis2Placement location = null)
        {
            var circleMoq = MakeMoq<IIfcCircle>();
            circleMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var circle = circleMoq.Object;
            circle.Radius = radius;
            circle.Position = location ?? IfcAxis2Placement3DMock();
            circleMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcCircle)));
            return circle;
        }
        public static IIfcCircle IfcCircle2dMock(double radius = 100, IIfcAxis2Placement location = null)
        {
            var circleMoq = MakeMoq<IIfcCircle>();
            circleMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var circle = circleMoq.Object;
            circle.Radius = radius;
            circle.Position = location ?? IfcAxis2Placement2DMock();
            circleMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcCircle)));
            return circle;
        }
        #endregion

        #region Ellipse Mocks
        public static IIfcEllipse IfcEllipse3dMock(double semi1 = 100, double semi2 = 50)
        {
            var ellipseMoq = MakeMoq<IIfcEllipse>();
            ellipseMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var ellipse = ellipseMoq.Object;
            ellipse.SemiAxis1 = semi1;
            ellipse.SemiAxis2 = semi2;
            ellipse.Position = IfcAxis2Placement3DMock();
            ellipseMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcEllipse)));
            return ellipse;
        }
        public static IIfcEllipse IfcEllipse2dMock(double semi1 = 100, double semi2 = 50)
        {
            var ellipseMoq = MakeMoq<IIfcEllipse>();
            ellipseMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var ellipse = ellipseMoq.Object;
            ellipse.SemiAxis1 = semi1;
            ellipse.SemiAxis2 = semi2;
            ellipse.Position = IfcAxis2Placement2DMock();
            ellipseMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcEllipse)));
            return ellipse;
        }
        #endregion

        #region Trimmed curve mocks
        public static IIfcTrimmedCurve IfcTrimmedCurve3dMock(IIfcCurve basisCurve = null, double trimParam1 = 0, double trimParam2 = 1, bool sense = true)
        {

            var trimMoq = MakeMoq<IIfcTrimmedCurve>();
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

            var trimMoq = MakeMoq<IIfcTrimmedCurve>();
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

        #region Composite Curve Mocks
        /// <summary>
        /// Default is a trimmed circle quadrant
        /// </summary>
        /// <param name="parent"></param>
        /// <returns></returns>
        public static IIfcCompositeCurveSegment IfcCompositeCurveSegment3dMock(IIfcCurve parent = null, bool sameSense = true, int entityLabel = 0)
        {
            var cCurveSegMoq = MakeMoq<IIfcCompositeCurveSegment>();
            cCurveSegMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            cCurveSegMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcCompositeCurveSegment)));
            cCurveSegMoq.SetupGet(v => v.EntityLabel).Returns(entityLabel);
            var cCurveSeg = cCurveSegMoq.Object;
            cCurveSeg.ParentCurve = parent ?? IfcTrimmedCurve3dMock(IfcMoq.IfcCircle3dMock(), 0, 90);
            cCurveSeg.SameSense = sameSense;
            return cCurveSeg;

        }
        public static IIfcCompositeCurve IfcCompositeCurve3dMock(IEnumerable<IIfcCompositeCurveSegment> segments = null)
        {
            var cCurveMoq = MakeMoq<IIfcCompositeCurve>();
            cCurveMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            cCurveMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcCompositeCurve)));
            var cCurve = cCurveMoq.Object;
            if (segments == null)
                cCurve.Segments.Add(IfcCompositeCurveSegment3dMock());
            else
                cCurve.Segments.AddRange(segments);
            cCurveMoq.SetupGet(v => v.NSegments).Returns(new IfcInteger(cCurve.Segments.Count));
            return cCurve;
        }

        public static IIfcCompositeCurve TypicalCompositeCurveMock(IXCurveFactory curveFactory, out double totalParametricLength, out double totalLength)
        {
            var lineLen1 = 10;
            var lineLen2 = 20;
            var circ1 = IfcMoq.IfcCircle3dMock(radius: 20);
            var circ2 = IfcMoq.IfcCircle3dMock(radius: 20, IfcMoq.IfcAxis2Placement3DMock(refDir: IfcMoq.IfcDirection3dMock(-1, 0, 0), loc: IfcMoq.IfcCartesianPoint3dMock(0, 40, 0)));
            var circ3 = IfcMoq.IfcCircle3dMock(radius: 20, IfcMoq.IfcAxis2Placement3DMock(loc: IfcMoq.IfcCartesianPoint3dMock(-40, 60, 0)));

            var line1 = IfcMoq.IfcLine3dMock(origin: IfcMoq.IfcCartesianPoint3dMock(x: 20, y: -lineLen1, z: 0), direction: IfcMoq.IfcDirection3dMock(x: 0, y: 1, z: 0));
            var lineSeg1 = IfcMoq.IfcTrimmedCurve3dMock(line1, trimParam1: 0, trimParam2: lineLen1);

            var line2 = IfcMoq.IfcLine3dMock(origin: IfcMoq.IfcCartesianPoint3dMock(x: -20, y: 40, z: 0), direction: IfcMoq.IfcDirection3dMock(x: 0, y: 1, z: 0));
            var lineSeg2 = IfcMoq.IfcTrimmedCurve3dMock(line2, trimParam1: 0, trimParam2: lineLen2);

            var arc1 = IfcMoq.IfcTrimmedCurve3dMock(circ1, trimParam2: 90);
            var arc2 = IfcMoq.IfcTrimmedCurve3dMock(circ2, trimParam2: 90, trimParam1: 0, sense: true);
            var arc3 = IfcMoq.IfcTrimmedCurve3dMock(circ3, trimParam2: 90);

            var x1 = curveFactory.Build(lineSeg1) as IXTrimmedCurve;
            var x2 = curveFactory.Build(arc1) as IXTrimmedCurve;
            var x3 = curveFactory.Build(arc2) as IXTrimmedCurve;
            var x4 = curveFactory.Build(lineSeg2) as IXTrimmedCurve;
            var x5 = curveFactory.Build(arc3) as IXTrimmedCurve;

            var paramLength1 = x1.LastParameter - x1.FirstParameter;
            var paramLength2 = x2.LastParameter - x2.FirstParameter;
            var paramLength3 = x3.LastParameter - x3.FirstParameter;
            var paramLength4 = x4.LastParameter - x4.FirstParameter;
            var paramLength5 = x5.LastParameter - x5.FirstParameter;

            totalParametricLength = paramLength1 + paramLength2 + paramLength3 + paramLength4 + paramLength5;
            totalLength = x1.Length + x2.Length + x3.Length + x4.Length + x5.Length;
            var seg1 = IfcMoq.IfcCompositeCurveSegment3dMock(lineSeg1, entityLabel: 1);
            var seg2 = IfcMoq.IfcCompositeCurveSegment3dMock(arc1, entityLabel: 2);
            var seg3 = IfcMoq.IfcCompositeCurveSegment3dMock(arc2, entityLabel: 3,sameSense: false);
            var seg4 = IfcMoq.IfcCompositeCurveSegment3dMock(lineSeg2, entityLabel: 4);
            var seg5 = IfcMoq.IfcCompositeCurveSegment3dMock(arc3, entityLabel: 5);

            return IfcMoq.IfcCompositeCurve3dMock(new[] { seg1, seg2, seg3, seg4, seg5 });
        }
        #endregion
    }
}
