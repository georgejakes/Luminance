// Luminance.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <thread>
#include <math.h>
#include <sstream>

using namespace std;
using namespace cv;
namespace fs = std::experimental::filesystem;

int *attendance;	// Attendance Table for Thread
vector<double> gLuminance;	// Video Luminance value storage vector

class VideoClass
{
	string path;
	double avgLuminance;
	int frameCount;
	double width;
	double height;
	vector<int> vframeDiff;
	int frameDiffSize;

	/* void CalculateStats()	//Implementation for Key-Frame detection: scrapped for challenge requirements
	{
		VideoCapture cap(path);
		if (cap.isOpened())
		{
			frameCount = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
			width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
			height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
			Size ksize = Size(9, 9);
			Size dsize = Size(round(width*0.25), round(height*0.25));
			if (frameCount == 0)
			{
				avgLuminance = 0;
			}

			Mat lastFrame;
			if (cap.read(lastFrame))
			{
				cvtColor(lastFrame, lastFrame, CV_BGR2GRAY);
				resize(lastFrame, lastFrame, dsize, 0, 0, CV_INTER_AREA);
				GaussianBlur(lastFrame, lastFrame, ksize, 0.0);
				int iter = 0;
				while (1)
				{
					Mat frame;
					bool bSuccess = cap.read(frame); // read a new frame from video
					if (!bSuccess) //if not success, break loop
					{
						break;
					}
					cvtColor(frame, frame, CV_BGR2GRAY);
					resize(frame, frame, dsize);
					GaussianBlur(frame, frame, ksize, 0.0);
					Mat diff;
					absdiff(frame, lastFrame, diff);
					int diffMag = countNonZero(diff);
					vframeDiff.push_back(diffMag);
				}
			}
		}
	} */

	/*
		> Calculates the Luminance of video in the class
		> Sets the avgLuminance variable of the class with calculated value
	*/
	void CalculateLuma()
	{
		vector<double> ls;		// Stores frame-by-frame luminance
		VideoCapture cap(path);	
		if (cap.isOpened())
		{
			frameCount = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
			width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
			height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
			while (1)
			{
				cap.set(CV_CAP_PROP_CONVERT_RGB, true);	// Specifies frames be returned in RGB
				Mat frame;
				vector<Mat> channels;
				bool bSuccess = cap.read(frame); // read a new frame from video
				if (!bSuccess) //if not success, break loop
				{
					break;
				}
				cvtColor(frame, frame, CV_RGB2YUV);	// Converts RGB to YUV, for Luma extraction, ie Y channel
				Scalar s = mean(frame);
				ls.push_back(s[0]);	//Extracts and stores mean summation of Luma in channel[0] of frame, ie 'Y'
			}
		}
		double sum_all = 0;
		for (double i : ls)
		{
			sum_all += i;
		}
		avgLuminance = sum_all / ls.size();	// Calculates average luminance over entire video
	}

public:
	/*	Class constructor
		@param: 
			p	-> path to video
		> Sets path variable
		> Calls CalculateLuma()
	*/
	VideoClass(string p)
	{
		path = p;
		frameDiffSize = 0;
		CalculateLuma();
	}

	// Retrieve the calculated average luminance of the video
	double getAvgLuminance()
	{
		return avgLuminance;
	}

	// Plays video
	void viewVideo()
	{
		VideoCapture cap(path);
		if (cap.isOpened())
		{
			while (1) // Temp code for play back
			{
				cap.set(CV_CAP_PROP_CONVERT_RGB, true);
				Mat frame;
				bool bSuccess = cap.read(frame); // read a new frame from video
				if (!bSuccess) //if not success, break loop
				{
				break;
				}
				//vector<Mat> channels;
				//split(frame, channels);
				imshow(path, frame);
				if (waitKey(30) == 27) //wait for 'esc' key press for 30 ms. If 'esc' key is pressed, break loop
				{
				cout << "esc key is pressed by user" << endl;
				break;
				}
			}
		}
	}

	/* float getFrameDiffMean()
	{
		float s = 0;
		for (int v : vframeDiff)
		{
		s += v;
		}
		return s/(float)vframeDiff.size();
	}

	float getFrameSd()
	{
		float mean = getFrameDiffMean();
		float s = 0;
		for (int v : vframeDiff)
		{
		s += pow(v - mean, 2);
		}
		s /= vframeDiff.size();
		return sqrt(s);
	} */
};



/*
	Used as part of a thread execution
	@params:
		path	-> Path to the video
		id		-> Index allocated in the global attendance table 
					for the particular thread running this function
	> Creates a VideoClass object
	> Retrieves the luminance from the VideoClass object 
	> Stores Luminance in gLuminance global vector variable

*/
void find_luma(string path, int id)
{
	VideoCapture cap(path);
	if (cap.isOpened())
	{
		if (cap.get(CV_CAP_PROP_FRAME_COUNT) >= 2)
		{
			VideoClass vid(path);
			double tempLum = vid.getAvgLuminance();
			cout << path << " Luminance avg:" << tempLum << endl;
			gLuminance.push_back(tempLum);
		}
	}
	attendance[id] = 0;	// Signals that the thread has finished its processes
}

/*
	Prints information regarding pre-computed luminance
	> Pre-requisite: Run find_luma() to populate the gLuminance Global vector
	> Calculates min, max, mean and median of contents within the gLuminance vector
	*/
void print_info()
{
	size_t sz = gLuminance.size();
	
	if (sz > 0)
	{
		stable_sort(gLuminance.begin(), gLuminance.end());
		double sum = 0, median;
		for (double t : gLuminance)
		{
			sum += t;
		}

		int middle;

		middle = (int)sz/2;
		if ((int)sz % 2 == 0)
		{
			median = (gLuminance.at(middle-1) + gLuminance.at(middle)) / 2.0;
		}
		else
		{
			median = gLuminance.at(middle);
		}
		cout << "Minimum Luminance: " << gLuminance.at(0) << endl;
		cout << "Maximum Luminance: " << gLuminance.at(sz - 1) << endl;
		cout << "Mean Luminance: " << sum / sz << endl;
		cout << "Median Luminance: " << median << endl;
	}	
}


/*
	Main function of the program:
	> Controls and verifies incoming inputs
	> Allocates threads for video processing
 */
int main(int argc, char** argv)
{
	string path = "ref";
	int MAX_THREAD_COUNT = 10;

	// Input checks for inconsistencies
	/*******************************************************************************************/
	if (argc != 3)
	{
		cerr << "Improper Input! Working with Default..." << endl << 
			"Use the following format: `ApplicationName folder_location thread_count`"<<endl;
	}
	else
	{
		path = argv[1];
		
		istringstream ss(argv[2]);
		if (!(ss >> MAX_THREAD_COUNT))
		{
			cerr << "Invalid number " << argv[2] << '\n';
			return 0;
		}
	}

	if (!fs::is_directory(path))
	{
		cerr << "Directory does not exist!";
		return 0;
	}

	if (MAX_THREAD_COUNT <= 0)
	{
		cerr << "Thread count cannot be 0 or less, resuming function with 1" << endl;
		MAX_THREAD_COUNT = 1;
	}
	/*******************************************************************************************/

	// Thread allocation and processing
	/*******************************************************************************************/
	vector<thread> thread_workers; // To store the threads dynamically during creation
	attendance = new int[MAX_THREAD_COUNT]; 

	//Initialize Attendance table to reflect that all slots are free
	for (int i = 0; i < MAX_THREAD_COUNT; i++)
	{
		attendance[i] = 0;
	}

	for (auto & p : fs::directory_iterator(path))
	{
		int flag = -1;
		while (1)
		{
			// If any slot is free in the attendance table, assign flag as the index of free slot
			for (int j = 0; j < MAX_THREAD_COUNT; j++)
			{
				if (attendance[j] == 0)
				{
					flag = j;
					break;
				}
			}
			// If flag has been allocated an attendance table index, set a thread for the specific index
			if (flag != -1)
			{
				thread_workers.push_back(thread(find_luma, p.path().string(), flag));
				attendance[flag] = 1;
				break;
			}
		}
	}

	// Wait for all threads to end
	for (thread &t : thread_workers)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	/*******************************************************************************************/

	print_info();	// Prints Calculated Information

	waitKey(0);
	return 0;
}

