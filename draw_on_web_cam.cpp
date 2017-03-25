#include <iostream>
#include <stdlib.h>  
#include <time.h>       
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


using namespace cv;
using namespace std;

//we will use this colors for The Win form app
#define blue  CV_RGB(0,0,255)
#define green CV_RGB(0,255,0)
#define red   CV_RGB(255,0,0)
#define white CV_RGB(255,255,255)
#define black CV_RGB(0,0,0)


int main(int argc, char** argv)
{
	VideoCapture cap(0); //capture the video from webcam


	if (!cap.isOpened())
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
	moveWindow("Control", 10, 500);

	int iLowH = 81;
	int iHighH = 123;

	int iLowS = 7;
	int iHighS = 165;

	int iLowV = 255;
	int iHighV = 255;

	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);

	int iLastX = -1;
	int iLastY = -1;

	//Capture a temporary image from the camera
	Mat imgTmp;
	cap.read(imgTmp);

	//Create a black image with the size as the camera output
	
	Mat imgLines = Mat::zeros(imgTmp.size(), CV_8UC3);
	imgLines = Scalar( 0, 0, 0);

	

	while (true)
	{
		Mat imgOriginal;

		bool bSuccess = cap.read(imgOriginal); // read a new frame from video


		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		Mat imgHSV;

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		Mat imgThresholded;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

																									  //morphological opening (removes small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//morphological closing (removes small holes from the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//Calculate the moments of the thresholded image
		Moments oMoments = moments(imgThresholded);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		int posX = 0;
		int posY = 0;
		


		if (dM10 / dArea >= 0 && dM10 / dArea < 1280 && dM01 / dArea >= 0 && dM01/ dArea < 1280 /* to control the limit */)
		{
			//calculate the position of the ball
			posX = (int)(dM10 / dArea);
			posY = (int)(dM01 / dArea);
		}

		// if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
		if (dArea > 10000)
		{
			double diff_X = iLastX - posX;
			double diff_Y = iLastY - posY;
			double magnitude = sqrt(pow(diff_X, 2) + pow(diff_Y, 2));

			// We want to draw a line only if its a valid position
			//if(iLastX>0 && iLastY>0 && posX>0 && posY>0)
			if (magnitude > 0 && magnitude < 100 && posX > 120 && posX<530)
			{
				// Draw a line from the previous point to the current point
				line(imgLines, cvPoint(posX, posY), cvPoint(iLastX, iLastY), red, 6, CV_AA);
			}
			

			iLastX = posX;
			iLastY = posY;
		}
		

		imshow("Thresholded Image", imgThresholded); //show the thresholded image

		imgOriginal = imgOriginal + imgLines;
		imshow("Original", imgOriginal); //show the original image

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			//save current frame
			vector<int> compression_params; //vector that stores the compression parameters of the image

			compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //specify the compression technique

			compression_params.push_back(98); //specify the compression quality
											  /* initialize random seed: */
			srand(time(NULL));

			/* generate secret number between 1 and 10: */
			int random_n = rand() % 10 - 1;
			char random = (char)random_n ;
			bool bSuccess = imwrite(("C:/Users/PC/Desktop/Image" + random + ".jpg"), imgOriginal, compression_params); //write the image to file
			if (!bSuccess)
			{
				cout << "error saving" << endl;
				waitKey(0);
			}
			break;
		}

		
	}

	return 0;
}

