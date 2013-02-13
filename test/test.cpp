#include <iostream>
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

int main()
{
    VideoCapture cap(-1); // open the default camera
    if(!cap.isOpened()) {  // check if we succeeded
        cout << "ERRORE MAS GRANDE";
       return -1;
}
    Mat edges;
    namedWindow("edges",CV_WINDOW_AUTOSIZE);
	cout << CV_WINDOW_AUTOSIZE;
    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, CV_BGR2GRAY);
        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
        imshow("edges", edges);
        if(waitKey(30) >= 0) break;
    }
return 0;
}
