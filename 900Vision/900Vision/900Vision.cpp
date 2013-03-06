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
	whiteBalance = 0,
	thresh = 15,
	colorSpec = 32;

void valCallback(int, void*){	
	if(saturation == 0)
		saturation = 1;

	vidCap.set(CV_CAP_PROP_BRIGHTNESS, brightness);
	vidCap.set(CV_CAP_PROP_CONTRAST, contrast);
	vidCap.set(CV_CAP_PROP_SATURATION, saturation);
	vidCap.set(CV_CAP_PROP_GAIN, gain);
	vidCap.set(CV_CAP_PROP_EXPOSURE, exposure-11);
	vidCap.set(CV_CAP_PROP_WHITE_BALANCE_BLUE_U, whiteBalance+2800);
}

double angle( Point pt1, Point pt2, Point pt0 )
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

int _tmain(int argc, _TCHAR* argv[])
{
	vector<double> ratios(2);
	//ratios[0] = 4.5; // High Inner
	//ratios[1] = 2.571428571428571; // Middle Inner
	ratios[0] = 3.6; // High Mid
	ratios[1] = 2.3; // Middle Mid

	// capture from video device #1
	vidCap.open(-1);

	vidCap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G'));

	/*
	vidCap.set(CV_CAP_PROP_BRIGHTNESS, brightness);
	vidCap.set(CV_CAP_PROP_CONTRAST, contrast);
	vidCap.set(CV_CAP_PROP_SATURATION, saturation);
	vidCap.set(CV_CAP_PROP_GAIN, gain);
	vidCap.set(CV_CAP_PROP_EXPOSURE, exposure-11);
	vidCap.set(CV_CAP_PROP_WHITE_BALANCE_BLUE_U, whiteBalance+2800);
	//*/
	//*
	vidCap.set(CV_CAP_PROP_BRIGHTNESS, 32);
	vidCap.set(CV_CAP_PROP_CONTRAST, 32);
	vidCap.set(CV_CAP_PROP_SATURATION, 50);
	vidCap.set(CV_CAP_PROP_GAIN, 0);
	vidCap.set(CV_CAP_PROP_EXPOSURE, -1);
	vidCap.set(CV_CAP_PROP_WHITE_BALANCE_BLUE_U, 6500);
	//*/
	vidCap.set(CV_CAP_PROP_FPS, 10);
	//vidCap.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	//vidCap.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	vidCap.set(CV_CAP_PROP_FRAME_WIDTH, 1280); // Use 8 FPS
	vidCap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	// create a window to display the images
	namedWindow("mainWindow", CV_WINDOW_AUTOSIZE);
	//namedWindow("configWindow", CV_WINDOW_NORMAL);
	/*
	createTrackbar("Brightness", "configWindow", &brightness, 255, valCallback);
	createTrackbar("Contrast", "configWindow", &contrast, 255, valCallback);
	createTrackbar("Saturation", "configWindow", &saturation, 255, valCallback);
	createTrackbar("Exposure", "configWindow", &exposure, 11, valCallback);
	createTrackbar("Gain", "configWindow", &gain, 255, valCallback);
	createTrackbar("White Balance", "configWindow", &whiteBalance, 3700, valCallback);
	createTrackbar("Color Spec", "configWindow", &colorSpec, 127, valCallback);
	//*/
	//createTrackbar("Threshold", "mainWindow", &thresh, 255);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	Mat camSrc, cFilter, cThresh, cDE;

	// Generate the kernel for morphology operations
	int de_size = 7;
	Mat deMat = getStructuringElement(
		MORPH_ELLIPSE, 
		Size(de_size, de_size), //ellipse inscribed in rect
		Point(-1, -1)); //anchor at center

	int key = -1;
	vector<Mat> bgr;
	while(key != 27) {
		key = waitKey(10);
		vidCap >> camSrc;

		try{
			//cvtColor(camSrc, camSrc, CV_YUV2BGR,4);

			vector<Mat> channels(camSrc.channels());
			split(camSrc, channels);

			/*
			string alpha = "0123456789";
			if(channels.size() > 0)
			for(int i = 0; i < channels.size(); ++i)
			{
			imshow(alpha.substr(i, 1), channels.at(i));
			channels.at(i).release();
			}
			//*/
			split(camSrc, bgr);
			cFilter = bgr.at(1) - (bgr.at(0) + bgr.at(2));
			threshold(cFilter, cThresh, 10, 255, CV_THRESH_BINARY);
			morphologyEx(cThresh, cDE, MORPH_CLOSE, deMat);//, Point(-1, -1), 5);

			Scalar color = Scalar(255,255,255);

			findContours(cDE, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
			//for( int i = 0; i< contours.size(); i++ )
			//	drawContours(cDE, contours, i, color, -1, 1, hierarchy, 0, Point());
			//findContours(cDE, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));

			vector<Rect> rect(contours.size());
			vector<RotatedRect> minRect(contours.size());

			for( int i = 0; i< contours.size(); i++ )
			{
				minRect[i] = minAreaRect(Mat(contours[i]));
				rect[i] = boundingRect(Mat(contours[i]));
				/*
				rect[i].x -= 10;
				rect[i].y -= 10;
				rect[i].width += 20;
				rect[i].height += 20;
				//*/
				float w = minRect[i].size.width, h = minRect[i].size.height;
				float ratio = (w > h) ? (w / h) : (h / w);
				double pm = 0.75;
				String type = "Unknown";
				bool isR = true;
				if(abs(ratio - ratios[0]) < pm)
					type = "High";
				else if(abs(ratio - ratios[1]) < pm)
					type = "Middle";
				else isR = false;

				if(isR) {
					//rectangle(cDE, rect[i].tl(), rect[i].br(), color, 1, 8);
					//rectangle(camSrc, rect[i].tl(), rect[i].br(), color, 1, 8);

					Point2f rect_points[4]; minRect[i].points( rect_points );
					for( int j = 0; j < 4; j++ ){
						line( cDE, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
						line( camSrc, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
					}

					string areaStr = static_cast<ostringstream*>( &(ostringstream() << minRect[i].size.area()) )->str();
					string angleStr = static_cast<ostringstream*>( &(ostringstream() << minRect[i].angle) )->str();
					string ratioStr = static_cast<ostringstream*>( &(ostringstream() << ratio) )->str();

					Point cOff = minRect[i].center;
					cOff.x += 25;
					putText(cDE, "Type:" + type, cOff, CV_FONT_HERSHEY_PLAIN, 1, color);
					putText(camSrc, "Type:" + type, cOff, CV_FONT_HERSHEY_PLAIN, 1, color);
					cOff.y += 25;
					putText(cDE, "Ratio:" + ratioStr, cOff, CV_FONT_HERSHEY_PLAIN, 1, color);
					putText(camSrc, "Ratio:" + ratioStr, cOff, CV_FONT_HERSHEY_PLAIN, 1, color);
				}
				//}
			}

			//imshow("BGR-Filter", cFilter);
			//imshow("Filter", cDE);
		}
		catch(Exception e)
		{
			std::cout << e.msg;
		}
		imshow("mainWindow", camSrc);
		camSrc.release();

		// Filter by removing pixels that have non-red presence
		// Requires thresholding afterwards, otherwise negatives appear
		/* RED   /		cFilter = bgr.at(2) - (bgr.at(0) + bgr.at(1)); /* RED   */
		/* GREEN /		cFilter = bgr.at(1) - (bgr.at(0) + bgr.at(2)); /* GREEN */
		/* BLUE  /		cFilter = bgr.at(0) - (bgr.at(1) + bgr.at(2)); /* BLUE  */

		// All values greater than 50 become white
		// Everything else is black
		//threshold(cFilter, cThresh, 10, 255, CV_THRESH_BINARY);
		//adaptiveThreshold(cFilter, cThresh, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 7, 0);
		// Removes small holes in image, erosion and dilation operation
		//morphologyEx(cThresh, cDE, MORPH_CLOSE, deMat);//, Point(-1, -1), 5);

		//findContours(cDE, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
		//vector<RotatedRect> minRect(contours.size());

		//merge(bgr, cDE);
		/*
		for( int i = 0; i< contours.size(); i++ )
		{
		minRect[i] = minAreaRect(Mat(contours[i]));
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
		drawContours(cDE, contours, i, color, 2, 1, hierarchy, 0, Point());
		Point2f rect_points[4]; minRect[i].points( rect_points );
		for( int j = 0; j < 4; j++ )
		line( cDE, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
		}
		//*/
	}
	std::cout << "Releasing Resources...";
	vidCap.release();
	camSrc.release();
	cFilter.release();
	//cThresh.release();
	//cDE.release();
	deMat.release();
	return 0;
}