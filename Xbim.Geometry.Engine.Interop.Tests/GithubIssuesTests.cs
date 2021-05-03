using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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
	}
}
