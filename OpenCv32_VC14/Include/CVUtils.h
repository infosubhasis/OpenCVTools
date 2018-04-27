
#ifndef CV_UTIL_H_INCLUDED
#define CV_UTIL_H_INCLUDED

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <Mil.h>
#include <omp.h>

using namespace cv;
using namespace std;




inline void displayImage(std::string windowName ,Mat& image)
{
	cv::namedWindow(windowName,cv::WINDOW_NORMAL);
	imshow(windowName,image);
	cv::waitKey(50);
}

inline void drawLine(cv::Mat& inputImage,cv::Point start,cv::Point end,int _thickness=3,int color=255)
{
		//cv::line(inputImage,start,end,cv::Scalar(color),2);
	for(int thickness = 0; thickness < _thickness+1 ; thickness++)
	{
		 // get a line iterator
		cv::LineIterator it(inputImage, start, end, 8);
		for(int i = 0; i < it.count; i++,it++)
			if ( i%2!=0 ) 
			{
				/*uchar * test = *it;
				cv::Point testP = it .pos();
				*test = 255;*/
				inputImage.at<uchar>(it .pos().y+thickness,it .pos().x)=color;
				//(*it)[0]  = 255;
			}
	}

}


inline cv::Mat ConvertMILtoCV(MIL_ID milImage)
{

	unsigned char *add = (unsigned char *)MbufInquire(milImage,M_HOST_ADDRESS,  M_NULL);

	unsigned int PitchB = (unsigned int)MbufInquire(milImage,M_PITCH_BYTE ,  M_NULL);

	int sizex = (int)MbufInquire(milImage, M_SIZE_X,M_NULL);
	int sizey = (int)MbufInquire(milImage, M_SIZE_Y,M_NULL);

	cv::Mat DstImage = cv::Mat::zeros(sizey,sizex, CV_8U);
	if(add!=NULL && DstImage.data!=NULL)
	{
		for(int i = 0;i<sizey;i++)
		{
			memcpy(DstImage.data+(i*DstImage.step),add+(i*PitchB),sizex);
		}
	}
	return DstImage;
}

inline MIL_ID ConvertCVtoMIL(MIL_ID MilSystem,cv::Mat CvImage)
{

	MIL_ID MilImage=0;
	
	if(CvImage.channels()==1 && CvImage.type()==0)
	{
		//MbufAlloc2d(MilSystem,CvImage.cols,CvImage.rows,8+M_UNSIGNED ,M_IMAGE + M_DISP ,&MilImage);
		MbufAllocColor(MilSystem,1,CvImage.cols,CvImage.rows,M_DEF_IMAGE_TYPE,M_IMAGE+M_PROC,&MilImage);
		unsigned char *add = (unsigned char *)MbufInquire(MilImage,M_HOST_ADDRESS,  M_NULL);
		unsigned int Pitch = (unsigned int)MbufInquire(MilImage,M_PITCH ,  M_NULL);
		unsigned int PitchB = (unsigned int)MbufInquire(MilImage,M_PITCH_BYTE ,  M_NULL);
		
		int sizex = (int)MbufInquire(MilImage, M_SIZE_X,M_NULL);
		int sizey = (int)MbufInquire(MilImage, M_SIZE_Y,M_NULL);

		if(add!=NULL && CvImage.data!=NULL)
		{
			for(int i = 0;i<sizey;i++)
			{
				memcpy(add+(i*PitchB),CvImage.data+(i*CvImage.step),sizex);
			}
		}

	}

	return MilImage;
}



#endif