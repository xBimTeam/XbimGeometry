using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Tests
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
        public static IIfcBooleanResult IfcBooleanResultMoq(IfcBooleanOperator boolOp = IfcBooleanOperator.UNION,
            double originX = 0, double originY = 0, double originZ = 0,
             double lenX = 10, double lenY = 20, double lenZ = 30,
            double displacementX = 0, double displacementY = 0, double displacementZ = 0)
        {
            var booleanResultMoq = MakeMoq<IIfcBooleanResult>();
            booleanResultMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcBooleanResult)));
            var booleanResult = booleanResultMoq.Object;
            var firstPosition = IfcAxis2Placement3DMock(loc: IfcCartesianPoint3dMock(x: originX, y: originY, z: originZ));
            var secondPosition = IfcAxis2Placement3DMock(loc: IfcCartesianPoint3dMock(x: displacementX + originX, y: displacementY + originY, z: displacementZ + originZ)); //consecutive to the first block in x direction
            booleanResult.FirstOperand = IfcBlockMoq(xLen: lenX, yLen: lenY, zLen: lenZ, position: firstPosition);
            booleanResult.SecondOperand = IfcBlockMoq(xLen: lenX, yLen: lenY, zLen: lenZ, position: secondPosition);
            booleanResult.Operator = boolOp;
            return booleanResult;
        }
        /// <summary>
        /// A boolean result with several levels of nested booleans of the same operation kind
        /// </summary>
        /// <param name="boolOp"></param>
        /// <returns></returns>
        public static IIfcBooleanResult IfcDeepBooleanResultMoq(IfcBooleanOperator boolOp = IfcBooleanOperator.UNION,
            int depth = 2, double displacement = 10)
        {
            var booleanResultMoq = MakeMoq<IIfcBooleanResult>();
            booleanResultMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcBooleanResult)));
            var booleanResult = booleanResultMoq.Object;
            var position = IfcAxis2Placement3DMock(loc: IfcCartesianPoint3dMock(x: 0, y: 0, z: 0));
            booleanResult.FirstOperand = IfcBlockMoq(xLen: displacement*2, yLen: 20, zLen: displacement*2, position: position);
            var offset = 0.0;
            
            var nextBooleanRes = booleanResult;
            for (int i = 0; i < depth; i++)
            {
                offset += displacement;
                var secondOpMoq = MakeMoq<IIfcBooleanResult>();
                nextBooleanRes.SecondOperand = secondOpMoq.Object;              
                secondOpMoq.SetupGet(x => x.ExpressType).Returns(metaData.ExpressType(typeof(IfcBooleanResult)));              
                position = IfcAxis2Placement3DMock(loc: IfcCartesianPoint3dMock(x: offset, y: offset, z: offset));
                secondOpMoq.Object.FirstOperand = IfcBlockMoq(xLen: displacement * 2, yLen: 20, zLen: displacement * 3, position: position);
                nextBooleanRes = secondOpMoq.Object;
            }
            offset += displacement;
            position = IfcAxis2Placement3DMock(loc: IfcCartesianPoint3dMock(x: offset, y: offset, z: offset));
            nextBooleanRes.SecondOperand = IfcBlockMoq(xLen: displacement * 2, yLen: 20, zLen: displacement * 3, position: position);
            return booleanResult;
        }
    }
}
