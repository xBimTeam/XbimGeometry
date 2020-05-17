using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.NetCore.Tests
{
    public static partial class IfcMoq
    {
        /// <summary>
        /// /Constructs two blocks the second one is displaced by the x,y,z values, default makes the blocks coincidental
        /// </summary>
        /// <param name="boolOp">Type of operation</param>
        /// <param name="displacementX"></param>
        /// <param name="displacementY"></param>
        /// <param name="displacementZ"></param>
        /// <returns></returns>
        public static IIfcBooleanResult IfcBooleanResultMoq(IfcBooleanOperator boolOp=IfcBooleanOperator.UNION,
             double lenX = 10, double lenY = 20, double lenZ = 30,
            double displacementX = 0, double displacementY = 0, double displacementZ = 0)
        {
            var booleanResultMoq = MakeMoq<IIfcBooleanResult>();
            booleanResultMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcBooleanResult)));
            var booleanResult = booleanResultMoq.Object;
            var firstPosition = IfcIfcAxis2Placement3DMock(loc: IfcCartesianPoint3dMock(x:0, y:0, z:0));
            var secondPosition = IfcIfcAxis2Placement3DMock(loc: IfcCartesianPoint3dMock(x: displacementX, y: displacementY, z: displacementZ)); //consecutive to the first block in x direction
            booleanResult.FirstOperand = IfcBlockMoq(xLen: lenX, yLen: lenY, zLen: lenZ, position: firstPosition);
            booleanResult.SecondOperand = IfcBlockMoq(xLen: lenX, yLen: lenY, zLen: lenZ, position: secondPosition);
            booleanResult.Operator = boolOp;
            return booleanResult;
        }
    }
}
