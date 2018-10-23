#include "OpenCVOperation.h"
#ifdef OPENCV
#include "opencv2\opencv.hpp"

#pragma comment(lib, CVLIB("core"))
#pragma comment(lib, CVLIB("imgproc"))

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
	if (type == 4)
	{
		Mat src = Mat(Size(w, h), CV_8UC4, pframe);
		flip(src, src, 1);
	}
}

void OpenCVOperation::GetBlackMask(cv::Mat src, cv::Mat &dst, int thresholdValue)
{
	cvtColor(src, dst, CV_BGRA2GRAY);
	threshold(dst, dst, thresholdValue, 255, ThresholdTypes::THRESH_BINARY);
}

void OpenCVOperation::Dilate(cv::Mat src, cv::Mat &dst, int dilateLevel, cv::MorphShapes type)
{
	if (dilateLevel != 0)
	{
		Mat element = getStructuringElement(type,
			cv::Size(2 * fabs(dilateLevel) + 1, 2 * fabs(dilateLevel) + 1),
			cv::Point(fabs(dilateLevel), fabs(dilateLevel)));
		if (dilateLevel > 0)
		{
			dilate(src, dst, element);
		}
		else if (dilateLevel < 0)
		{
			erode(src, dst, element);
		}
	}
	else
	{
		src.copyTo(dst);
	}
}

cv::Point OpenCVOperation::GetRectCenterPoint(cv::Rect rect)
{
	Point re;
	re.x = rect.x + rect.width / 2;
	re.y = rect.y + rect.height / 2;
	return re;
}

cv::Rect OpenCVOperation::GetMaskContentRect(cv::Mat src, int haveEdge)
{
	Rect re;
	int mL = 0;
	int mT = 0;
	int mR = 0;
	int mB = 0;
	double val = 0;
	for (int i = 0; i < src.cols; i++)
	{
		cv::minMaxLoc(src.col(i), NULL, &val);
		if (val > 0)
		{
			mL = i - haveEdge;
			(std::max)(mL, 0);
			break;
		}
	}
	val = 0;
	for (int i = 0; i < src.rows; i++)
	{
		cv::minMaxLoc(src.row(i), NULL, &val);
		if (val > 0)
		{
			mT = i - haveEdge;
			(std::max)(mT, 0);
			break;
		}
	}
	val = 0;
	for (int i = src.cols - 1; i >= 0; i--)
	{
		cv::minMaxLoc(src.col(i), NULL, &val);
		if (val > 0)
		{
			mR = i + haveEdge;
			(std::min)(mR, src.cols - 1);
			break;
		}
	}
	val = 0;
	for (int i = src.rows - 1; i >= 0; i--)
	{
		cv::minMaxLoc(src.row(i), NULL, &val);
		if (val > 0)
		{
			mB = i + haveEdge;
			(std::min)(mB, src.rows - 1);
			break;
		}
	}
	re.x = mL;
	re.y = mT;
	re.width = mR - mL;
	re.height = mB - mT;
	return re;
}

cv::Point OpenCVOperation::GetMaskContentCenterPoint(cv::Mat src)
{
	return GetRectCenterPoint(GetMaskContentRect(src, 0));
}

void OpenCVOperation::AddNoiseToImage(cv::Mat src, cv::Mat &dst, NoiseType noiseType, int value, int value2)
{
	switch (noiseType)
	{
	case NoiseType::NT_Gaussian:
	{
		Mat noise = Mat(Size(src.cols, src.rows), src.type(), OCD_Black);
		Mat tmp = Mat(Size(src.cols, src.rows), src.type(), OCD_Black);
		cv::randn(noise, value, value2);
		tmp = src + noise;
		tmp.copyTo(dst);
	}
		break;
	case NoiseType::NT_SaltPepper:
		break;
	}
}

void OpenCVOperation::CopyToEx(cv::Mat src, cv::Mat & dst, cv::Rect roi)
{
	dst = Mat(Size(roi.width, roi.height), src.type(), OCD_Black);
	int w = src.cols;
	int h = src.rows;
	if (roi.x > w || roi.y > h)
	{
		goto Error;
	}
	{
		int blendX = (std::min)(roi.x, 0);
		int blendY = (std::min)(roi.y, 0);
		int blendX2 = (std::max)(roi.x + roi.width - w, 0);
		int blendY2 = (std::max)(roi.y + roi.height - h, 0);
		if (blendX == 0 && blendY == 0 && blendX2 == 0 && blendY2 == 0)
		{
			src(roi).copyTo(dst);
		}
		else
		{
			int tmpW = w - blendX + blendX2;
			int tmpH = h - blendY + blendY2;
			int tmpRoiX = 0;
			int tmpRoiY = 0;
			roi.x < 0 ? tmpRoiX = fabs(roi.x) : tmpRoiX = 0;
			roi.y < 0 ? tmpRoiY = fabs(roi.y) : tmpRoiY = 0;
			Mat tmpSrc = Mat(Size(tmpW, tmpH), src.type(), OCD_Black);
			src.copyTo(tmpSrc(Rect(tmpRoiX, tmpRoiY, src.cols, src.rows)));

			int newRoiX = 0;
			int newRoiY = 0;
			roi.x < 0 ? newRoiX = 0 : newRoiX = roi.x;
			roi.y < 0 ? newRoiY = 0 : newRoiY = roi.y;
			tmpSrc(Rect(newRoiX, newRoiY, roi.width, roi.height)).copyTo(dst);
		}
	}
Error:
	return;
}
#endif