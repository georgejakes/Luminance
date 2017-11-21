// Luminance.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <math.h>
#include <sstream>

using namespace std;
using namespace cv;
namespace fs = std::experimental::filesystem;

int *attendance;


class VideoClass
{
	string path;
	double avgLuminance;
	int frameCount;
	double width;
	double height;
	vector<int> vframeDiff;
	int frameDiffSize = 0;
	/* void CalculateStats()
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

	void CalculateLuma()
	{
		vector<double> ls;
		VideoCapture cap(path);
		if (cap.isOpened())
		{
			frameCount = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
			width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
			height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
			while (1)
			{
				cap.set(CV_CAP_PROP_CONVERT_RGB, true);
				Mat frame;
				vector<Mat> channels;
				bool bSuccess = cap.read(frame); // read a new frame from video
				if (!bSuccess) //if not success, break loop
				{
					break;
				}
				cvtColor(frame, frame, CV_RGB2YUV);
				Scalar s = mean(frame);
				ls.push_back(s[0]);
			}
		}
		double sum_all = 0;
		for (double i : ls)
		{
			sum_all += i;
		}
		avgLuminance = sum_all / ls.size();
	}

public:
	VideoClass(string p)
	{
		path = p;
		CalculateLuma();
	}

	/* void setAvgLuminance(double val)
	{
		avgLuminance = val;
	} */

	double getAvgLuminance()
	{
		return avgLuminance;
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
};

vector<double> gLuminance;

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
	attendance[id] = 0;
}

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



int main(int argc, char** argv)
{
	string path = "ref";
	int MAX_THREAD_COUNT = 10;
	if (argc != 3)
	{
		cerr << "Improper Input! Working with Default..." << endl << 
			"Use the following format: `ApplicationName folder_location thread_count`"<<endl;
	}
	else
	{
		path = argv[1];
		if (!fs::is_directory(path))
		{
			cerr << "Directory does not exist!";
			return 0;
		}
		istringstream ss(argv[2]);
		if (!(ss >> MAX_THREAD_COUNT))
		{
			cerr << "Invalid number " << argv[2] << '\n';
			return 0;
		}
		
		if (MAX_THREAD_COUNT <= 0)
		{
			cerr << "Thread count cannot be 0 or less, resuming function with 1" << endl;
			MAX_THREAD_COUNT = 1;
		}
	}

	vector<thread> thread_workers; // To store the threads dynamically during creation
	attendance = new int[MAX_THREAD_COUNT]; 
	//Initialize Attendance
	for (int i = 0; i < MAX_THREAD_COUNT; i++)
	{
		attendance[i] = 0;
	}

	for (auto & p : fs::directory_iterator(path))
	{
		int flag = -1;
		while (1)
		{
			for (int j = 0; j < MAX_THREAD_COUNT; j++)
			{
				if (attendance[j] == 0)
				{
					flag = j;
					break;
				}
			}
			if (flag != -1)
			{
				thread_workers.push_back(thread(find_luma, p.path().string(), flag));
				attendance[flag] = 1;
				break;
			}
		}
	}

	for (thread &t : thread_workers)
	{
		if (t.joinable())
		{
			t.join();
		}
	}

	print_info();

	waitKey(0);
	return 0;
}

