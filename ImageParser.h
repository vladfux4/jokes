#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <algorithm>

#define PI 3.14159265
#define RAD 0.0174533

class ImageParser
{
public:
	ImageParser(cv::Mat const & img);
	~ImageParser();

	float kAproxymateEps = 0.1f;
	float kBeelineEps = 10.0f;
	int kMoreBlackSize = 2;
	float kTrianglesFilter = 0.3f;

	void setCoef(float kAproxymateEps, float kBeelineEps, int kMoreBlackSize, float kTrianglesFilter);

	void perform();
	std::vector< std::vector<cv::Vec6f> > const * GetTriangles();
	std::vector<std::vector<cv::Point> > const * GetContours();	
private:
	cv::Mat source;
	std::vector<std::vector<cv::Point> > * contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat contoursIndex;
	cv::Mat angles;
	std::vector<bool> performedContours;
	std::vector< std::vector<cv::Vec6f> > * contoursTriangleList;

	void init();
	bool isTraingleLess(cv::Point a, cv::Point b, cv::Point c, uchar threshold = 127, float limit = 0.5f);
	bool findPairContourIndex(int contourIndex, const cv::Point & point, int & pairIndex, cv::Point & result);
	void SetIndexMat();
	void SetAngleMat();
	void findContours();
	void removeBeeline(std::vector<cv::Point> & contour, float offset = 5.0f);
	void moreBlack(cv::Mat & img, const int size);

	template<class T>
	void addPointsToMiddle(T & a, T & b, float offset, std::vector<T>& out)
	{
		if (decDistance(a, b) >= offset) {
			T middle = getMiddlePoint(a, b);

			addPointsToMiddle<T>(a, middle, offset, out);
			out.push_back(middle);
			addPointsToMiddle<T>(middle, b, offset, out);
		}
	}

	template<class T>
	T getMiddlePoint(T & a, T & b)
	{
		return T((a.x + b.x) / 2.0f, (a.y + b.y) / 2.0f);
	}

	float decDistance(cv::Point a1, cv::Point a2)
	{
		return sqrtf(powf((a2.x - a1.x), 2) + powf((a2.y - a1.y), 2));
	}

	template <class T>
	bool isInRange(const T & value, const T & min, const T & max) {
		return ((value <= max) && (value >= min));
	}
};

