using System;
using Xbim.Common.Geometry;

namespace Xbim.ModelGeometry.Scene
{
	[Obsolete]
	public interface IXbimScene
	{
		void Close();
		TransformGraph Graph { get; }
		XbimLOD LOD { get; set; }
	}
}
