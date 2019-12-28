using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.ModelGeometry.Scene;

namespace GeometryTests
{
    [TestClass]
    public class AAA_DebugTest
    {
        [TestMethod]
        public void AAA_RunOneFIle()
        {
            using (IfcStore s = IfcStore.Open(@"C:\Users\Claudio\OneDrive\Benghi\2019 - migliorie e bug\2019 12 - Termolog\Esempio TERMOLOG.ifc"))
            {
                var v = new TransformGraph(s);
                var storey = s.Instances[30] as IIfcBuildingStorey;
                v.AddProduct(storey);
                var v2 = v[storey];
                var v3 = v2.WorldMatrix();
            }
        }
    }
}
