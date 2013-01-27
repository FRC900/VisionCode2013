#include<stdio.h>
#include<iostream>
#include<opencv2/opencv.hpp>
#include<boost/thread/thread.hpp>

#define HIGH_GOAL 0
#define MID_GOAL 1
#define LOW_GOAL 2
#define PYRAMID_GOAL 3

using namespace std;
using namespace cv;

bool threadRunning, mainRunning;

void threadCam(Mat* cDis);
void findTargets(Mat &img);
void classifyTargets(vector< vector<Point> > &contours);

int main(int argc, char **argv){
	cout << "Starting..." << endl;
	mainRunning = true;

	int key = -1;
	//namedWindow("src", CV_WINDOW_AUTOSIZE);

	//namedWindow("cFilter", CV_WINDOW_AUTOSIZE);
        namedWindow("cDisplay", CV_WINDOW_AUTOSIZE);

	Mat cDisplay;
	cout << "Starting thread..." << endl;
	boost::thread camThread(threadCam, &cDisplay);
	cout << "Thread started...";

	cout << "Waiting for thread...";
	while(!threadRunning){
		cout << ".";
		waitKey(5);
	}
	cout << "DONE!" << endl;

	while(key != 27){ // ESC
		cout << "dis..." << endl;
		imshow("cDisplay", cDisplay);
		key = waitKey(30);
		cout.flush();
	}
	cout << "Key: " << key << endl;

	mainRunning = false;

	camThread.join();

	cDisplay.release();
	return 0;
}

void threadCam(Mat* cDis){
	cout << "Opening Camera...";
	VideoCapture vidCap(0);
	if(vidCap.isOpened()){
		cout << "DONE!" << endl;

		cout << "Adjusting camera properties...";
		vidCap.set(CV_CAP_PROP_FPS, 8);
		vidCap.set(CV_CAP_PROP_BRIGHTNESS, 1);
//		vidCap.set(CV_CAP_PROP_CONTRAST, 70);
//		vidCap.set(CV_CAP_PROP_SATURATION, 50);
		//vidCap.set(CV_CAP_PROP_FRAME_WIDTH, 1280); // Use 8 FPS
		//vidCap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);
		cout << "DONE!" << endl;

		Mat camSrc, cFilter, cThresh, cDE;
		vector<Mat> bgr;

		// Generate the kernel for morphology operations
		int de_size = 7;
		Mat deMat = getStructuringElement(
				MORPH_ELLIPSE, 
				Size(de_size, de_size), //ellipse inscribed in rect
				Point(-1, -1)); //anchor at center

		vidCap >> *cDis;
		threadRunning = true;

		while(mainRunning){
			cout << "cap..." << endl;
			vidCap >> camSrc;
			split(camSrc, bgr);

			// Filter by removing pixels that have non-red presence
			// Requires thresholding afterwards, otherwise negatives appear
/* RED   */		cFilter = bgr.at(2) - (bgr.at(0) + bgr.at(1)); /* RED   */
/* GREEN /		cFilter = bgr.at(1) - (bgr.at(0) + bgr.at(2)); /* GREEN */
/* BLUE  /		cFilter = bgr.at(0) - (bgr.at(1) + bgr.at(2)); /* BLUE  */

			// All values greater than 50 become white
			// Everything else is black
			threshold(cFilter, cThresh, 50, 255, CV_THRESH_BINARY);
			
			// Removes small holes in image, erosion and dilation operation
			morphologyEx(cThresh, cDE, MORPH_CLOSE, deMat);//, Point(-1, -1), 3);

			bgr.at(2) += cDE;

			merge(bgr, cDE);

			*cDis = cDE.clone();
		}
		cout << "Releasing Resources...";
		vidCap.release();
		camSrc.release();
		cFilter.release();
		cThresh.release();
		cDE.release();
		deMat.release();
		cout << "DONE!" << endl;
	} else{
		cout << "ERR!" << endl;
	}

	threadRunning = false;
}

void findTargets(Mat &img){
  // Locates all regions of interest in the image
  Mat cannyImg;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  
  // find all contours in a binary image
  // full hierarchy of nested contours
  // returns only end-points of contours
  // no offset
  findContours(img, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

  // Find the bounding-box statistics for the important contours
  
}

void classifyTargets(vector<vector<Point> > &contours){
  // Classifies targets as HIGH_GOAL, MID_GOAL, LOW_GOAL
}

