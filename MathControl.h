#ifndef MATH_CONTROL_H
#define MATH_CONTROL_H

#include <math.h>
#define PI 3.14159265358979323846

class MathControl
{
public:
	float CalculateAngle(float x1, float y1, float x2, float y2)
	{
		return (float)(atan2(y1 - y2, x1 - x2) * 180 / PI);
	}
};
extern MathControl g_mathControl;

#endif