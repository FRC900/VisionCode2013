#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/thread/thread.hpp>
#include <list>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include <time.h>

#define SERVERPORT 1130

#define HIGH_GOAL 0
#define MID_GOAL 1
#define LOW_GOAL 2
#define PYRAMID_GOAL 3

#define CONTOURMAX 2
#define CONTOUR_MIN_AREA 20

using namespace std;
using namespace cv;

bool threadRunning, mainRunning;

int m_area;
Rect m_rect;
time_t started;
time_t timer;

void threadCam(Mat* cDis);
void findTargets(Mat &img);
void threadServer(Mat* cDis);

int main(int argc, char **argv){

  m_rect = Rect();
  
  cout << "Starting..." << endl;
  mainRunning = true;
  
  int key = -1;
  //namedWindow("src", CV_WINDOW_AUTOSIZE);
  //namedWindow("cFilter", CV_WINDOW_AUTOSIZE);
  namedWindow("cDisplay", CV_WINDOW_AUTOSIZE);
  
  Mat cDisplay;
  cout << "Starting thread..." << endl;
  boost::thread camThread(threadCam, &cDisplay);
  boost::thread serverThread(threadServer, &cDisplay);
  cout << "Thread started...";
  
  cout << "Waiting for thread...";
  while(!threadRunning){
    cout << ".";
    waitKey(5);
  }
  cout << "DONE!" << endl;
  
  while(key != 27){ // ESC
   //cout << "dis..." << endl;
    imshow("cDisplay", cDisplay);
    key = waitKey(30);
    cout.flush();
  }
  cout << "Key: " << key << endl;
  
  mainRunning = false;
  
  camThread.join();
  serverThread.join();
  
  cDisplay.release();
  return 0;
}

void threadServer(Mat* cDis){
  int connfd = 0;
  struct sockaddr_in serv_addr;
  
  // Create the file descriptor
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  
  memset(&serv_addr, '0', sizeof(serv_addr));

  // Setup the address parameters
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(SERVERPORT);

  // Bind socket to the port
  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  
  // Mark as passive socket (used to accept connections)
  listen(listenfd, 10);

  while(1){
    // A blocking call, waits for a message/connection
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    // Respond with an image
    //write(connfd, cDis->data, cDis->rows * cDis->cols * cDis->elemSize());
    
    // Respond with rectangle area
    cout << "before the sprintf" << endl;
    char* sArea = new char[50];
    sprintf(sArea, "%d", m_area);
    write(connfd, sArea, sizeof(sArea));

    // Print image dimensions
    //cout << cDis->rows << " AND " << cDis->cols << endl;

    // Close the connection, and wait for another
    close(connfd);
    sleep(1);
  }

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
	  
	  timer = time(NULL);
	  time_t last = timer;
	  int num_frames = 0;

	  while(mainRunning){
	    //cout << "cap..." << endl;
	    vidCap >> camSrc;
	    num_frames++;
	    timer = time(&timer);

	    if((timer-last) > 10){
	      last = timer;
	      cout << "FPS: " << double(num_frames) / double(10) << endl;
	      cout << "Rows: " << camSrc.rows << " Cols: " << camSrc.cols << endl;
	      num_frames = 0;
	    }
	    
	    split(camSrc, bgr);
	    
	    // Filter by removing pixels that have non-red presence
	    // Requires thresholding afterwards, otherwise negatives appear
	    /* RED   /		cFilter = bgr.at(2) - (bgr.at(0) + bgr.at(1)); /* RED   */
	    /* GREEN */		cFilter = bgr.at(1) - (bgr.at(0) + bgr.at(2)); /* GREEN */
	    /* BLUE  /		cFilter = bgr.at(0) - (bgr.at(1) + bgr.at(2)); /* BLUE  */
	    
	    // All values greater than 50 become white
	    // Everything else is black
	    threshold(cFilter, cThresh, 10, 255, CV_THRESH_BINARY);
	    
	    // Removes small holes in image, erosion and dilation operation
	    morphologyEx(cThresh, cDE, MORPH_CLOSE, deMat);//, Point(-1, -1), 3);
	    findTargets(cDE);
	    
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

  // Find the bounding-box statistics for the contours
  vector<Rect> boundRect(contours.size());
  vector<vector<Point> > contours_poly(contours.size());

  m_area = 0;
  m_rect = Rect();

  // Approximate polygons and create bounding rectangles
  //TODO: Only do this for most prominent bboxes, filter small ones
  for(int i=0; i<contours.size(); i++){
    approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true);
    boundRect[i] = boundingRect(Mat(contours_poly[i]));
    if(boundRect[i].area() > m_area){
      m_rect = boundRect[i];
      m_area = boundRect[i].area();
    }
  }

  // Draw the bbox for each contour
  //for(int i=0; i<COUNTERMAX; i++){
    Scalar color = Scalar(255, 255, 255);
    //drawContours( img, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point());
    // draw rect, provide top-left and bottomr-right corners
    rectangle(img, m_rect.tl(), m_rect.br(), color, 2, 8, 0);
    char* text = new char[50];
    sprintf(text, "(%d, %d)", m_rect.height, m_rect.width);
    putText(img, text, m_rect.br(), FONT_HERSHEY_COMPLEX_SMALL, 0.8, color, 1, CV_AA);
    //}
}

