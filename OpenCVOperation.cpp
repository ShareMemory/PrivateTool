#include "OpenCVOperation.h"
#include "opencv2\opencv.hpp"

using namespace cv;

OpenCVOperation g_openCVOperation;

OpenCVOperation::OpenCVOperation()
{

}

OpenCVOperation::~OpenCVOperation()
{

}

void OpenCVOperation::Flip(unsigned char *pframe, int w, int h, int type)
{
	if (type = 4)
	{
		Mat src = Mat(Size(w, h), CV_8UC4, pframe);
		flip(src, src, 1);
	}
}