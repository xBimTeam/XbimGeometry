using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Common.Geometry;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;

namespace Xbim.Geometry.Engine.Interop.Tests
{
	[TestClass]
	public class GithubIssuesTests
	{
		[TestMethod]
		public void Github_Issue_281()
		{
			// this file resulted in a stack-overflow exception due to precision issues in the data.
			// We have added better exception management so that the stack-overflow is not thrown any more, 
			// however the voids in the wall are still not computed correctly.
			//
			using (var m = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3()))
			{
				m.LoadStep21("TestFiles\\Github\\Github_issue_281_minimal.ifc");
				var c = new Xbim3DModelContext(m);
				c.CreateContext(null, false);

				// todo: 2021: add checks so that the expected openings are correctly computed.
			}
		}

		[TestMethod]
		public void Github_Issue_382_IntersectionIndependence()
		{
			DirectoryInfo d = new DirectoryInfo(".");
			Debug.WriteLine(d.FullName);
			XbimGeometryEngine geoEngine = new XbimGeometryEngine();
			var Inputs = new[] { false, true };


			// wether we perform an other intersection or not, there should be one resulting solid in simpleElement
            foreach (var input in Inputs)
            {
				string probElementBrep = File.ReadAllText(@".\TestFiles\Github\Issue382\problem-element.brep");
				string roomBrep = File.ReadAllText(@".\TestFiles\Github\Issue382\room.brep");
				string victimElementBrep = File.ReadAllText(@".\TestFiles\Github\Issue382\victim-element.brep");
				IXbimSolid probemElement = (IXbimSolid)geoEngine.FromBrep(probElementBrep);
				IXbimSolid room = (IXbimSolid)geoEngine.FromBrep(roomBrep);
				IXbimSolid simpleElement = (IXbimSolid)geoEngine.FromBrep(victimElementBrep);
				IXbimSolidSet intersection = null;
				if (input)
					intersection = room.Intersection(probemElement, 0); //COMMENT THIS LINE TO CHANGE NEXT INTERSECTION RESULT.
				IXbimSolidSet corrupted = room.Intersection(simpleElement, 0);
				Assert.AreEqual(1, corrupted.Count);
			}
		}
	}
}
