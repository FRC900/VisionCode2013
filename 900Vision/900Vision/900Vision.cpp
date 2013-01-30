// 900Vision.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include <tbb\tbb.h>

using namespace cv;
using namespace std;

VideoCapture vidCap;
int brightness = 128,
		contrast = 32,
		saturation = 32,
		exposure = 5,
		gain = 64,
		thresh = 15;

void valCallback(int, void*){	
	if(saturation == 0)
		saturation = 1;

	vidCap.set(CV_CAP_PROP_BRIGHTNESS, brightness);
	vidCap.set(CV_CAP_PROP_CONTRAST, contrast);
	vidCap.set(CV_CAP_PROP_SATURATION, saturation);
	vidCap.set(CV_CAP_PROP_GAIN, gain);
	vidCap.set(CV_CAP_PROP_EXPOSURE, exposure-11);
}

int _tmain(int argc, _TCHAR* argv[])
{
	// capture from video device #1
	vidCap.open(-1);
	/*
	vidCap.set(CV_CAP_PROP_BRIGHTNESS, brightness);
	vidCap.set(CV_CAP_PROP_CONTRAST, contrast);
	vidCap.set(CV_CAP_PROP_SATURATION, saturation);
	vidCap.set(CV_CAP_PROP_GAIN, gain);
	vidCap.set(CV_CAP_PROP_EXPOSURE, exposure-11);
	//*/
	//*
	vidCap.set(CV_CAP_PROP_BRIGHTNESS, 50);
	vidCap.set(CV_CAP_PROP_CONTRAST, 40);
	vidCap.set(CV_CAP_PROP_SATURATION, 40);
	vidCap.set(CV_CAP_PROP_GAIN, 60);
	vidCap.set(CV_CAP_PROP_EXPOSURE, -11);
	//*/
	vidCap.set(CV_CAP_PROP_FPS, 8);
	vidCap.set(CV_CAP_PROP_FRAME_WIDTH, 1280); // Use 8 FPS
	vidCap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	// create a window to display the images
	namedWindow("mainWindow", CV_WINDOW_AUTOSIZE);
	/*
	createTrackbar("Brightness", "mainWindow", &brightness, 255, valCallback);
	createTrackbar("Contrast", "mainWindow", &contrast, 255, valCallback);
	createTrackbar("Saturation", "mainWindow", &saturation, 255, valCallback);
	createTrackbar("Exposure", "mainWindow", &exposure, 11, valCallback);
	createTrackbar("Gain", "mainWindow", &gain, 255, valCallback);
	//*/
	//createTrackbar("Threshold", "mainWindow", &thresh, 255);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	RNG rng(12345);
	Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );

	Mat camSrc, cFilter, cThresh, cDE;
	vector<Mat> bgr;

	// Generate the kernel for morphology operations
	int de_size = 7;
	Mat deMat = getStructuringElement(
			MORPH_ELLIPSE, 
			Size(de_size, de_size), //ellipse inscribed in rect
			Point(-1, -1)); //anchor at center

	int key = -1;
	while(key != 27) {
		key = waitKey(10);
		vidCap >> camSrc;
		//cvtColor(camSrc, camSrc, CV_YCrCb2BGR);

		split(camSrc, bgr);

			// Filter by removing pixels that have non-red presence
			// Requires thresholding afterwards, otherwise negatives appear
/* RED   /		cFilter = bgr.at(2) - (bgr.at(0) + bgr.at(1)); /* RED   */
/* GREEN */		cFilter = bgr.at(1) - (bgr.at(0) + bgr.at(2)); /* GREEN */
/* BLUE  /		cFilter = bgr.at(0) - (bgr.at(1) + bgr.at(2)); /* BLUE  */

		// All values greater than 50 become white
		// Everything else is black
		threshold(cFilter, cThresh, 10, 255, CV_THRESH_BINARY);
		//adaptiveThreshold(cFilter, cThresh, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 7, 0);
		// Removes small holes in image, erosion and dilation operation
		morphologyEx(cThresh, cDE, MORPH_CLOSE, deMat);//, Point(-1, -1), 5);
		
		findContours(cDE, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
		vector<RotatedRect> minRect(contours.size());

		merge(bgr, cDE);

		for( int i = 0; i< contours.size(); i++ )
		{
			minRect[i] = minAreaRect(Mat(contours[i]));
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
			drawContours(cDE, contours, i, color, 2, 1, hierarchy, 0, Point());
			Point2f rect_points[4]; minRect[i].points( rect_points );
			for( int j = 0; j < 4; j++ )
				line( cDE, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
		}
		
      	imshow("mainWindow", cDE);
      	//imshow("B", bgr.at(0));
      	//imshow("G", bgr.at(1));
      	//imshow("R", bgr.at(2));
	}
	std::cout << "Releasing Resources...";
	vidCap.release();
	camSrc.release();
	cFilter.release();
	cThresh.release();
	cDE.release();
	deMat.release();
	return 0;
}