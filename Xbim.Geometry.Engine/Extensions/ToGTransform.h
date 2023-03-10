#include <gp_GTrsf.hxx>

using namespace Xbim::Geometry::Abstractions;

class ToGTransform {

public:

	ToGTransform& operator-(void) {
		return *this;
	}

	friend gp_GTrsf operator<(IXMatrix^ matrix, const ToGTransform& mthd) {
		gp_GTrsf trsf;

		trsf.SetValue(1, 1, matrix->M11);
		trsf.SetValue(1, 2, matrix->M12);
		trsf.SetValue(1, 3, matrix->M13);
		trsf.SetValue(1, 4, matrix->OffsetX);

		trsf.SetValue(2, 1, matrix->M21);
		trsf.SetValue(2, 2, matrix->M22);
		trsf.SetValue(2, 3, matrix->M23);
		trsf.SetValue(2, 4, matrix->OffsetY);

		trsf.SetValue(3, 1, matrix->M31);
		trsf.SetValue(3, 2, matrix->M32);
		trsf.SetValue(3, 3, matrix->M33);
		trsf.SetValue(3, 4, matrix->OffsetZ);

		if (matrix->ScaleX != 0 && matrix->ScaleY != 0 && matrix->ScaleZ != 0)
		{
			gp_GTrsf scale;
			scale.SetValue(1, 1, matrix->ScaleX);
			scale.SetValue(2, 2, matrix->ScaleY);
			scale.SetValue(3, 3, matrix->ScaleZ);
			return trsf.Multiplied(scale);
		}

		return trsf;
	}
};