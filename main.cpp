#include<stdio.h>
#include<iostream>
#include<opencv2/opencv.hpp>
#include<boost/thread/thread.hpp>

using namespace std;
using namespace cv;

bool threadRunning, mainRunning;

void threadCam(Mat* cDis);
void findTargets(Mat &img);

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

		int de_size = 3;
		Mat deMat = getStructuringElement(
				2, 
				Size(2*de_size+1, 2*de_size+1),
				Point(de_size, de_size));

		vidCap >> *cDis;
		threadRunning = true;

		while(mainRunning){
			cout << "cap..." << endl;
			vidCap >> camSrc;
			split(camSrc, bgr);

/* RED   */		cFilter = bgr.at(2) - (bgr.at(0) + bgr.at(1)); /* RED   */
/* GREEN /		cFilter = bgr.at(1) - (bgr.at(0) + bgr.at(2)); /* GREEN */
/* BLUE  /		cFilter = bgr.at(0) - (bgr.at(1) + bgr.at(2)); /* BLUE  */
			threshold(cFilter, cThresh, 50, 255, CV_THRESH_BINARY);
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
	Mat cannyImg;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	
	findContours(img, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
}
