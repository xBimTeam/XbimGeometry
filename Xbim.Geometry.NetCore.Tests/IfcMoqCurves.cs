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
        public static IModel IfcModelMock(double millimetre = 1, double precision = 1e-5, double radianFactor = 1)
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

        public static IIfcAxis2Placement3D IfcIfcAxis2Placement3DMock(IIfcDirection axis = null, IIfcDirection refDir = null, IIfcCartesianPoint loc = null)
        {
            var axis3dMoq = MakeMoq<IIfcAxis2Placement3D>();
            var axis3d = axis3dMoq.Object;
            axis3d.Axis = axis??IfcDirection3dMock();
            axis3d.RefDirection = refDir;
            axis3d.Location = loc ?? IfcCartesianPoint3dMock();
            return axis3d;
        }

        public static IIfcAxis2Placement2D IfcIfcAxis2Placement2DMock(IIfcDirection refDir = null, IIfcCartesianPoint loc = null)
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
            circle.Position = location ?? IfcIfcAxis2Placement3DMock();
            circleMoq.SetupGet(v => v.ExpressType).Returns(metaData.ExpressType(typeof(IfcCircle)));
            return circle;
        }
        public static IIfcCircle IfcCircle2dMock(double radius = 100, IIfcAxis2Placement location = null)
        {
            var circleMoq = MakeMoq<IIfcCircle>();
            circleMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var circle = circleMoq.Object;
            circle.Radius = radius;
            circle.Position = location ?? IfcIfcAxis2Placement2DMock();
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
            ellipse.Position = IfcIfcAxis2Placement3DMock();
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
            ellipse.Position = IfcIfcAxis2Placement2DMock();
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
            cCurveSeg.ParentCurve = parent ?? IfcTrimmedCurve3dMock(IfcMoq.IfcCircle3dMock(), trimParam2: Math.PI / 2.0);
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
        #endregion
    }
}
