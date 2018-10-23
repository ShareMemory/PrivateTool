#ifndef OPENCV_OPERATION_H
#define OPENCV_OPERATION_H
#ifdef OPENCV
#include "OpenCVColorDefinitions.h"
#include "opencv2\opencv.hpp"

#define CV_VERSION_LIB CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

#ifdef _DEBUG
#define cvLIB(name) "opencv_" name CV_VERSION_LIB "d"
#else
#define CVLIB(name) "opencv_" name CV_VERSION_LIB
#endif

enum MouseType {
	MT_NoButton = 0,
	MT_LeftButton,
	MT_MiddleButton,
	MT_RightButton,
	MT_VScroll
};

enum MouseStatus {
	MS_Move = 0,
	MS_LeftDown = 1,
	MS_RightDown = 2,
	MS_LeftUp = 4,
	MS_RightUp = 5
};

enum NoiseType {
	NT_Unknown = 0,
	NT_Gaussian = 1,
	NT_SaltPepper = 2,

};

class OpenCVOperation
{
public:
	OpenCVOperation();
	~OpenCVOperation();
	void Flip(unsigned char *pframe, int w, int h, int type);
	void GetBlackMask(cv::Mat src, cv::Mat &dst, int thresholdValue = 1);
	void Dilate(cv::Mat src, cv::Mat &dst, int dilateLevel, cv::MorphShapes type);
	cv::Point GetRectCenterPoint(cv::Rect rect);
	cv::Rect GetMaskContentRect(cv::Mat src, int haveEdge = 1);
	cv::Point GetMaskContentCenterPoint(cv::Mat src);
	void AddNoiseToImage(cv::Mat src, cv::Mat &dst, NoiseType noiseType, int value, int value2);
	void CopyToEx(cv::Mat src, cv::Mat &dst, cv::Rect roi);
};

extern OpenCVOperation g_openCVOperation;
#endif
#endif