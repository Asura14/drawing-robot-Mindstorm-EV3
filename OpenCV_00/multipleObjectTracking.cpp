#include <sstream>
#include <string>
#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Object.h"

#define PI 3.14159265

Mat cameraFeed;
bool currentlyTrackingRobot = false;
bool courseIsCompleted = false;
cv::Point2f robotPosition_front;
cv::Point2f robotPosition_rear;
cv::Point2f robotCenter;
cv::Point2f nextCheckpoint;

int nextCheckpointIndex = 0;
const float ANGLE_DIFFERENCE_TOLERANCE = 5.0f;
const float XY_DIFFERENCE_TOLERANCE = 30.0f;

//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN_FRONT = 0;
int H_MAX_FRONT = 256;
int S_MIN_FRONT = 0;
int S_MAX_FRONT = 256;
int V_MIN_FRONT = 0;
int V_MAX_FRONT = 256;
int H_MIN_REAR = 0;
int H_MAX_REAR = 256;
int S_MIN_REAR = 0;
int S_MAX_REAR = 256;
int V_MIN_REAR = 0;
int V_MAX_REAR = 256;

/*
int H_MIN_DEFAULT = 0;
int H_MAX_DEFAULT = 256;
int S_MIN_DEFAULT = 49;
int S_MAX_DEFAULT = 217;
int V_MIN_DEFAULT = 41;
int V_MAX_DEFAULT = 256;
*/

//default capture width and height
//const int FRAME_WIDTH = 640;
//const int FRAME_HEIGHT = 480;

// high resolution:
const int FRAME_WIDTH = 1280;
const int FRAME_HEIGHT = 960;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;

//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2_front = "Thresholded Image FRONT";
const string windowName2_rear = "Thresholded Image REAR";
const string windowName3 = "After Morphological Operations";
const string trackbarFrontWindowName = "Front Circle Tracking";
const string trackbarRearWindowName = "Rear Circle Tracking";

//The following for canny edge detection
Mat dst, detected_edges;
Mat src, src_gray;
int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
const char* window_name_front = "Edge Map FRONT";
const char* window_name_rear = "Edge Map REAR";

// ---------------------------------------------- C�DIGO BASEADO EM VCOM (INI) ----------------------------------------------
const cv::Scalar GREEN = cv::Scalar(0, 255, 0);
const cv::Scalar RED = cv::Scalar(0, 0, 255);
const cv::Scalar BLUE = cv::Scalar(255, 0, 0);

std::vector<cv::Point2f> corners;
std::vector<cv::Point2f> course;
std::vector<bool> courseCheckpoints;

cv::Mat roi; // Region of Interest for Homography

float sizeW = 0; // RoI width
float sizeH = 0; // RoI width

cv::Size sizeHomo(0, 0);

cv::Mat warpedImage;

// Create a vector of destination points
std::vector<cv::Point2f> cornersDst;

cv::Mat h;

double distance(cv::Point2f p0, cv::Point2f p1)
{
	double dX0 = p0.x, dY0 = p0.y, dX1 = p1.x, dY1 = p1.y;
	return std::sqrt((dX1 - dX0)*(dX1 - dX0) + (dY1 - dY0)*(dY1 - dY0));
}

//Mouse callback to select the 4 initial corners
void selectCorners(int event, int x, int y, int flags, void* param)
{
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		// draw circle on clicked spots
		circle(cameraFeed, cv::Point(x, y), 3, RED, 5, CV_AA);
		cv::imshow("Original Image", cameraFeed);
		if (corners.size() < 4)
		{
			corners.push_back(cv::Point2f(x, y));
		}
	}
}

//Mouse callback to select drawing course
void selectCourse(int event, int x, int y, int flags, void* param)
{
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		cout << "adding course point (x = " << x << ", y = " << y << ")" << endl;
		// draw circle on clicked spots
		circle(roi, cv::Point(x, y), 3, BLUE, 5, CV_AA);
		cv::imshow("REGION OF INTEREST", roi);
		if (course.size() < 4)
		{
			course.push_back(cv::Point2f(x, y));
			courseCheckpoints.push_back(false);
		}
	}
}

// ---------------------------------------------- C�DIGO BASEADO EM VCOM (FIM) ----------------------------------------------

void on_trackbar( int, void* )
{//This function gets called whenever a
	// trackbar position is changed

}

string intToString(int number){
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void createTrackbars(){
	//  ------- Front -------
	//create window for trackbars
	namedWindow(trackbarFrontWindowName,0);
	//create memory to store trackbar name on window
	char TrackbarName_front[50];
	sprintf_s( TrackbarName_front, "H_MIN_FRONT", H_MIN_FRONT);
	sprintf_s( TrackbarName_front, "H_MAX_FRONT", H_MAX_FRONT);
	sprintf_s( TrackbarName_front, "S_MIN_FRONT", S_MIN_FRONT);
	sprintf_s( TrackbarName_front, "S_MAX_FRONT", S_MAX_FRONT);
	sprintf_s( TrackbarName_front, "V_MIN_FRONT", V_MIN_FRONT);
	sprintf_s( TrackbarName_front, "V_MAX_FRONT", V_MAX_FRONT);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH),
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->

	/**/
	createTrackbar("H_MIN_FRONT", trackbarFrontWindowName, &H_MIN_FRONT, H_MAX_FRONT, on_trackbar);
	createTrackbar("H_MAX_FRONT", trackbarFrontWindowName, &H_MAX_FRONT, H_MAX_FRONT, on_trackbar);
	createTrackbar("S_MIN_FRONT", trackbarFrontWindowName, &S_MIN_FRONT, S_MAX_FRONT, on_trackbar);
	createTrackbar("S_MAX_FRONT", trackbarFrontWindowName, &S_MAX_FRONT, S_MAX_FRONT, on_trackbar);
	createTrackbar("V_MIN_FRONT", trackbarFrontWindowName, &V_MIN_FRONT, V_MAX_FRONT, on_trackbar);
	createTrackbar("V_MAX_FRONT", trackbarFrontWindowName, &V_MAX_FRONT, V_MAX_FRONT, on_trackbar);
	/*
	// tentativa de atribuir valores por defeito nas trackbars:
	createTrackbar( "H_MIN", trackbarWindowName, &H_MIN_DEFAULT, H_MAX, on_trackbar );
	createTrackbar( "H_MAX", trackbarWindowName, &H_MAX_DEFAULT, H_MAX, on_trackbar );
	createTrackbar( "S_MIN", trackbarWindowName, &S_MIN_DEFAULT, S_MAX, on_trackbar );
	createTrackbar( "S_MAX", trackbarWindowName, &S_MAX_DEFAULT, S_MAX, on_trackbar );
	createTrackbar( "V_MIN", trackbarWindowName, &V_MIN_DEFAULT, V_MAX, on_trackbar );
	createTrackbar( "V_MAX", trackbarWindowName, &V_MAX_DEFAULT, V_MAX, on_trackbar );
	*/

	//  ------- Rear -------
	namedWindow(trackbarRearWindowName, 0);
	char TrackbarName_rear[50];
	sprintf_s(TrackbarName_rear, "H_MIN_REAR", H_MIN_REAR);
	sprintf_s(TrackbarName_rear, "H_MAX_REAR", H_MAX_REAR);
	sprintf_s(TrackbarName_rear, "S_MIN_REAR", S_MIN_REAR);
	sprintf_s(TrackbarName_rear, "S_MAX_REAR", S_MAX_REAR);
	sprintf_s(TrackbarName_rear, "V_MIN_REAR", V_MIN_REAR);
	sprintf_s(TrackbarName_rear, "V_MAX_REAR", V_MAX_REAR);
	createTrackbar("H_MIN_REAR", trackbarRearWindowName, &H_MIN_REAR, H_MAX_REAR, on_trackbar);
	createTrackbar("H_MAX_REAR", trackbarRearWindowName, &H_MAX_REAR, H_MAX_REAR, on_trackbar);
	createTrackbar("S_MIN_REAR", trackbarRearWindowName, &S_MIN_REAR, S_MAX_REAR, on_trackbar);
	createTrackbar("S_MAX_REAR", trackbarRearWindowName, &S_MAX_REAR, S_MAX_REAR, on_trackbar);
	createTrackbar("V_MIN_REAR", trackbarRearWindowName, &V_MIN_REAR, V_MAX_REAR, on_trackbar);
	createTrackbar("V_MAX_REAR", trackbarRearWindowName, &V_MAX_REAR, V_MAX_REAR, on_trackbar);
}

void drawObject(vector<Object> theObjects,Mat &frame){

	for(int i =0; i<theObjects.size(); i++){

	cv::circle(frame,cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()),10,cv::Scalar(0,0,255));
	cv::putText(frame,intToString(theObjects.at(i).getXPos())+ " , " + intToString(theObjects.at(i).getYPos()),cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()+20),1,1,Scalar(0,255,0));
	cv::putText(frame,theObjects.at(i).getType(),cv::Point(theObjects.at(i).getXPos(),theObjects.at(i).getYPos()-30),1,2,theObjects.at(i).getColor());
	}
}

void morphOps(Mat &thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle
	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);

	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);
}
void trackFilteredObject(Mat threshold,Mat HSV, Mat &cameraFeed, Point2f &coords)
{
	vector <Object> detectedObjects;
	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if(numObjects<MAX_NUM_OBJECTS)
		{
			for (int index = 0; index >= 0; index = hierarchy[index][0])
			{
				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;
				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if(area>MIN_OBJECT_AREA)
				{
					Object object;

					object.setXPos(moment.m10/area);
					object.setYPos(moment.m01/area);

					detectedObjects.push_back(object);

					objectFound = true;

				}
				else objectFound = false;
			}
			//let user know you found an object
			if(objectFound ==true)
			{
				//draw object location on screen
				drawObject(detectedObjects,cameraFeed);
			}
			if (detectedObjects.size() == 1)
			{
				coords.x = detectedObjects[0].getXPos();
				coords.y = detectedObjects[0].getYPos();
				//cout << "robot pos 1 - x = " << robotPosition1.x << ", y = " << robotPosition1.y << "\n";
			}
			else
				currentlyTrackingRobot = false;
		}
		else putText(cameraFeed,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
	}
}

void robot()
{
	// cout << "\n\ntwo detected objects!\n\n";
	//currentlyTrackingRobot = true;
	//robotPosition1.x = detectedObjects[0].getXPos();
	//robotPosition1.y = detectedObjects[0].getYPos();
	//robotPosition2.x = detectedObjects[1].getXPos();
	//robotPosition2.y = detectedObjects[1].getYPos();
	robotCenter.x = (robotPosition_front.x + robotPosition_rear.x) / 2.0f;
	robotCenter.y = (robotPosition_front.y + robotPosition_rear.y) / 2.0f;

	// cout << "robot pos 2 - x = " << robotPosition2.x << ", y = " << robotPosition2.y << "\n";
	cout << "\n\nrobot center - x = " << robotCenter.x << ", y = " << robotCenter.y << "\n";


	float angle1 = atan((robotPosition_front.y - robotPosition_rear.y) / (robotPosition_front.x - robotPosition_rear.x)) * 180/PI;
	cout << "\nAngle 1 = " << angle1 << endl;

	float nextAngle = -1;

	// convert to angle with horizontal
	// angle1 -= 90;
	// angle2 -= 90;	

	if (!courseIsCompleted) {
		nextCheckpoint.x = course[nextCheckpointIndex].x;
		nextCheckpoint.y = course[nextCheckpointIndex].y;
		cout << "nextCheckpoint x = " << nextCheckpoint.x << ", y = " << nextCheckpoint.y << "\n";
		nextAngle = atan((nextCheckpoint.y - robotCenter.y) / (nextCheckpoint.x - robotCenter.x)) * 180 / PI;

		cout << "nextAngle = " << nextAngle << endl;
		float distanceToCheckpoint = distance(robotCenter, nextCheckpoint);
		float angleDifference = nextAngle - angle1;
		cout << "angle difference = " << angleDifference << endl;

		
		// Print instructions for robot
		if (distanceToCheckpoint < XY_DIFFERENCE_TOLERANCE)
		cout << "REACHED_CHECKPOINT" << endl << "STOP" << endl;
		else
		if (abs(angleDifference) < ANGLE_DIFFERENCE_TOLERANCE)
		cout << "GO_FORWARD";
		else if (angleDifference > 0)
		cout << "ROTATE_RIGHT";
		else
		cout << "ROTATE_LEFT";
		

		// check to see if robot within tolerance range for next spot
		if ((abs(robotCenter.x - nextCheckpoint.x) < XY_DIFFERENCE_TOLERANCE)
			&& (abs(robotCenter.y - nextCheckpoint.y) < XY_DIFFERENCE_TOLERANCE)) {

			courseCheckpoints[nextCheckpointIndex] = true; // validates reached checkpoint
			cout << " --- Hit checkpoint i = " << nextCheckpointIndex << endl;
			if (nextCheckpointIndex == course.size() - 1)
				courseIsCompleted = true; // course is complete once all the course's checkpoints are ran
			else
			{
				nextCheckpointIndex++;
				nextCheckpoint = course[nextCheckpointIndex];
			}
		}
	}
}


int main(int argc, char* argv[])
{
	//if we would like to calibrate our filter values, set to true.
	bool calibrationMode = true;

	bool homographyMode = true;
	bool homographyCalculated = false;
	bool courseSet = false;

	//Matrix to store each frame of the webcam feed

	Mat threshold_front;
	Mat threshold_rear;
	Mat HSV_front;
	Mat HSV_rear;

	if(calibrationMode){
		//create slider bars for HSV filtering
		createTrackbars();
	}
	//video capture object to acquire webcam feed
	VideoCapture capture;

	//open capture object at location zero (default location for webcam)
	capture.open(0);

	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);

	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	waitKey(1000);
	while(1){
		//store image to matrix
		capture.read(cameraFeed);

		if (homographyCalculated == true)
			src = roi;
		else
			src = cameraFeed;

  		if( !src.data )
  		{ return -1; }

		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed,HSV_front,COLOR_BGR2HSV);
		cvtColor(cameraFeed, HSV_rear, COLOR_BGR2HSV);

		// ------------- MODO DE HOMOGRAFIA -------------
		if (homographyMode == true)
		{
			if (!homographyCalculated) { // se ainda n�o foi definida a homografia, calcular com base nos 4 pontos do rato
				std::cout << "'Original' image window:\nClick on four corners -- top left first and bottom left last -- and then hit any Key" << std::endl;

				imshow(windowName, cameraFeed);

				// acquire points for homography
				// Set the callback function for any mouse event
				cv::setMouseCallback("Original Image", selectCorners, 0);
				cv::waitKey(0);

				//Set the size for the region of interest
				sizeW = distance(corners[0], corners[1]);
				sizeH = distance(corners[1], corners[2]);


				sizeHomo.width = sizeW;
				sizeHomo.height = sizeH;

				warpedImage = cv::Mat::zeros(sizeHomo, CV_8UC3);

				cornersDst.push_back(cv::Point2f(0, 0));
				cornersDst.push_back(cv::Point2f(sizeHomo.width - 1, 0));
				cornersDst.push_back(cv::Point2f(sizeHomo.width - 1, sizeHomo.height - 1));
				cornersDst.push_back(cv::Point2f(0, sizeHomo.height - 1));

				// Calculate the homography
				h = cv::findHomography(corners, cornersDst);

				// Warp source image to destination
				warpPerspective(cameraFeed, warpedImage, h, sizeHomo);

				warpedImage.copyTo(roi);

				// Show REGION OF INTEREST
				cv::imshow("REGION OF INTEREST", roi);

				homographyCalculated = true;
				cv::setMouseCallback("Original Image", NULL, NULL);
			}
			else // se a homografia j� foi definida
			{
				if (!courseSet) { // se o percurso de desenho ainda n�o foi desenhado
					std::cout << "\n\n'REGION OF INTEREST' window: \nClick on the points for the robot's course -- and then hit any Key\n\n" << std::endl;
					cv::imshow("REGION OF INTEREST", roi);

					// Set the callback function for any mouse event
					cv::setMouseCallback("REGION OF INTEREST", selectCourse, 0);
					cv::waitKey(0);
					courseSet = true;
					cv::setMouseCallback("REGION OF INTEREST", NULL, NULL);
				}

				// Warp source image to destination
				warpPerspective(cameraFeed, warpedImage, h, sizeHomo);
				warpedImage.copyTo(roi);

				// -------------- detec��o de objetos (INI) --------------

				// ------- FRONT -------
				//if in calibration mode, we track objects based on the HSV slider values.
				cvtColor(roi, HSV_front, COLOR_BGR2HSV);
				inRange(HSV_front, Scalar(H_MIN_FRONT, S_MIN_FRONT, V_MIN_FRONT), Scalar(H_MAX_FRONT, S_MAX_FRONT, V_MAX_FRONT), threshold_front);
				morphOps(threshold_front);
				imshow(windowName2_front, threshold_front); // thresholded image

				//the folowing for canny edge detec
				/// Create a matrix of the same type and size as src (for dst)
				dst.create(src.size(), src.type());
				/// Convert the image to grayscale
				cvtColor(src, src_gray, CV_BGR2GRAY);
				/// Create a window
				namedWindow(window_name_front, CV_WINDOW_AUTOSIZE);
				/// Create a Trackbar for user to enter threshold
				createTrackbar("Min Threshold:", window_name_front, &lowThreshold, max_lowThreshold);
				/// Show the image
				trackFilteredObject(threshold_front, HSV_front, roi, robotPosition_front);

				// ------- REAR -------
				//if in calibration mode, we track objects based on the HSV slider values.
				cvtColor(roi, HSV_rear, COLOR_BGR2HSV);
				inRange(HSV_rear, Scalar(H_MIN_REAR, S_MIN_REAR, V_MIN_REAR), Scalar(H_MAX_REAR, S_MAX_REAR, V_MAX_REAR), threshold_rear);
				morphOps(threshold_rear);
				imshow(windowName2_rear, threshold_rear); // thresholded image

												//the folowing for canny edge detec
												/// Create a matrix of the same type and size as src (for dst)
				dst.create(src.size(), src.type());
				/// Convert the image to grayscale
				cvtColor(src, src_gray, CV_BGR2GRAY);
				/// Create a window
				namedWindow(window_name_rear, CV_WINDOW_AUTOSIZE);
				/// Create a Trackbar for user to enter threshold
				createTrackbar("Min Threshold:", window_name_rear, &lowThreshold, max_lowThreshold);
				/// Show the image
				trackFilteredObject(threshold_rear, HSV_rear, roi, robotPosition_rear);

				robot();

				// -------------- detec��o de objetos (FIM) --------------

				// desenha os 4 cantos do percurso de desenho
				// (azul ainda n�o atingidos; verde j� atingidos)
				if (course.size() != courseCheckpoints.size())
				{
					cerr << "error, different sized vectors" << endl;
					return -1;
				}
				for (int i = 0; i < course.size(); i++)
					if (courseCheckpoints[i] == true)
						circle(roi, cv::Point(course[i].x, course[i].y), 3, GREEN, 5, CV_AA);
					else
						circle(roi, cv::Point(course[i].x, course[i].y), 3, BLUE, 5, CV_AA);

				// check distance between robot and next course spot

				// Show original image
				imshow(windowName, cameraFeed); 
				// Show REGION OF INTEREST
				cv::imshow("REGION OF INTEREST", roi);
			}

		} 


		//show frames
		//imshow(windowName2,threshold);

		imshow(windowName,cameraFeed);
		//imshow(windowName1,HSV);

		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		waitKey(30);
	}
	return 0;
}
