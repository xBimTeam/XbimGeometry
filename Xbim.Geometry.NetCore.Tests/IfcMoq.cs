using Moq;
using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Common;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.MeasureResource;

namespace Xbim.Geometry.NetCore.Tests
{
    public class IfcMoq
    {
        
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
            var dirMoq = new Mock<IIfcDirection>() { DefaultValue = DefaultValue.Mock,  DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            dirMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var dir = dirMoq.Object;
            dir.DirectionRatios.AddRange(new IfcReal[] { x, y, z});
            dirMoq.SetupGet(v => v.X).Returns(dir.DirectionRatios[0]);
            dirMoq.SetupGet(v => v.Y).Returns(dir.DirectionRatios[1]);
            dirMoq.SetupGet(v => v.Z).Returns(dir.DirectionRatios[2]);
            return dir;
        }
        public static IIfcCartesianPoint IfcCartesianPoint2dMock(double x = 10, double y = 20)
        {
            var cpMoq = new Mock<IIfcCartesianPoint>() { DefaultValue = DefaultValue.Mock,  DefaultValueProvider = new MoqDefaultBehaviourProvider() }
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
            { DefaultValue = DefaultValue.Mock,  DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            cpMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(3));
            var cp = cpMoq.Object;
            cp.Coordinates.AddRange(new IfcLengthMeasure[] { x, y, z });
            cpMoq.SetupGet(v => v.X).Returns(cp.Coordinates[0]);
            cpMoq.SetupGet(v => v.Y).Returns(cp.Coordinates[1]);
            cpMoq.SetupGet(v => v.Z).Returns(cp.Coordinates[2]);
            return cp;
        }

        public static IIfcLine IfcLine2dMock(IIfcCartesianPoint origin = null, IIfcVector direction = null)
        {
            var lineMoq = new Mock<IIfcLine>() 
            { DefaultValue = DefaultValue.Mock, DefaultValueProvider = new MoqDefaultBehaviourProvider() }
            .SetupAllProperties();
            lineMoq.SetupGet(v => v.Dim).Returns(new IfcDimensionCount(2));
            var line = lineMoq.Object;
            line.Pnt = origin ?? IfcCartesianPoint2dMock();
            line.Dir = direction ?? IfcVector2dMock();
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
            return line;
        }
    }
}
