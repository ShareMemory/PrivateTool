#ifndef OPENCV_OPERATION_H
#define OPENCV_OPERATION_H

class OpenCVOperation
{
public:
	OpenCVOperation();
	~OpenCVOperation();
	void Flip(unsigned char *pframe, int w, int h, int type);
};

extern OpenCVOperation g_openCVOperation;
#endif