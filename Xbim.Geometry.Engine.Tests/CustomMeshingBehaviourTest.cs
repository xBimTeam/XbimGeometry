using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;

namespace GeometryTests
{
    [TestClass]
    public class CustomMeshingBehaviourTest
    {
        [TestMethod]
        [DeploymentItem(@"SolidTestFiles\OneWallTwoWindows.ifc")]
        [DeploymentItem(@"x64\", "x64")]
        [DeploymentItem(@"x86\", "x86")]
        public void Do()
        {
            var f = new DirectoryInfo(".");
            Debug.WriteLine(f.FullName);
            
            using (var model = IfcStore.Open("OneWallTwoWindows.ifc"))
            {
                var geomContext = new Xbim3DModelContext(model);
                geomContext.CustomMeshingBehaviour += beh;
                geomContext.CreateContext();
                model.SaveAs("asxbim.xbim");
                model.Close();
            }
            Debug.WriteLine(f.FullName);
        }

        private Xbim3DModelContext.MeshingBehaviourResult beh(int elementId, int typeId, ref double linearDeflection, ref double angularDeflection)
        {
            var shortTypeId = Convert.ToInt16(typeId);
            //if (shortTypeId == 453) // skip wall standard case
            //    return Xbim3DModelContext.MeshingBehaviourResult.Skip;
            if (shortTypeId == 667) // skip windows
                return Xbim3DModelContext.MeshingBehaviourResult.Skip;
            return Xbim3DModelContext.MeshingBehaviourResult.Default;
        }
    }
}
