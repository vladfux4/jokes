/**
int main(int argc, char** argv)
{
	Mat image = imread("PATH", CV_LOAD_IMAGE_GRAYSCALE);
	Mat drawing(image.size(), CV_8UC3);

	ImageParser * parser = new ImageParser(image);
	parser->setCoef(0.1f,10.0f,2,0.3f);
	parser->perform();

	drawContours(drawing, *parser->GetContours(), -1, Scalar(0, 0, 255));

	auto contourTriangleList = parser->GetTriangles();
	vector<Point> pt(3);
	for (auto triangleList = contourTriangleList->begin(); triangleList < contourTriangleList->end(); triangleList++)
	{
		for (size_t i = 0; i < triangleList->size(); i++)
		{
			Vec6f t = triangleList->at(i);
			pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
			pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
			pt[2] = Point(cvRound(t[4]), cvRound(t[5]));

			line(drawing, pt[0], pt[1], Scalar(255, 255, 0), 1, 8, 0);
			line(drawing, pt[1], pt[2], Scalar(255, 255, 0), 1, 8, 0);
			line(drawing, pt[2], pt[0], Scalar(255, 255, 0), 1, 8, 0);
		}
	}
	imshow("img", drawing);
	waitKey(0);
	return 0;
}
**/


#include "ImageParser.h"
#include <string>

using namespace std;
using namespace cv;

ImageParser::ImageParser(Mat const & img)
{
	source = img;

	if (!source.data) // Check for invalid input
	{
		throw string("No image");
	}
	init();

}


ImageParser::~ImageParser()
{
}

std::vector< std::vector<cv::Vec6f> >  const * ImageParser::GetTriangles()
{
	return contoursTriangleList;
}

std::vector<std::vector<cv::Point>> const * ImageParser::GetContours()
{
	return contours;
}

void ImageParser::init()
{
	contours = new vector<vector<Point> >();
	contoursIndex = Mat::zeros(source.size(), CV_8U);
	angles = Mat::zeros(source.size(), CV_32F);
	contoursTriangleList = new vector<vector<Vec6f> >();
}

void ImageParser::setCoef(float kAproxymateEps, float kBeelineEps, int kMoreBlackSize, float kTrianglesFilter)
{
	this->kAproxymateEps = kAproxymateEps;
	this->kBeelineEps = kBeelineEps;
	this->kMoreBlackSize = kMoreBlackSize;
	this->kTrianglesFilter = kTrianglesFilter;
}

void ImageParser::perform()
{
	findContours();
	performedContours.resize(contours->size());
	performedContours[contours->size() - 1] = true;

	for (auto contour = contours->begin(); contour < contours->end(); contour++)
	{
		approxPolyDP(*contour, *contour, kAproxymateEps, true);
		removeBeeline(*contour, kBeelineEps);
	}

	SetIndexMat();
	SetAngleMat();
	moreBlack(source, kMoreBlackSize);
	
	for (auto contour = contours->begin(); contour < contours->end() - 1; contour++)
	{
		if (!performedContours[contour - contours->begin()])
		{
			auto currentPoint = contour->begin();
			Point rawPairPoint;
			int pairContourIndex;
			findPairContourIndex(contour - contours->begin(), *currentPoint, pairContourIndex, rawPairPoint);

			Subdiv2D * subdiv = new Subdiv2D(Rect(0, 0, source.cols, source.rows)); //@TODO Fix Crah on delete;
			vector<Vec6f> * triangleList = new vector<Vec6f>(); //@TODO Fix Crah on delete;
			vector<Vec6f> resultTriangleList;

			if (pairContourIndex != -1) {
				if (pairContourIndex != contour - contours->begin())
				{
					performedContours[pairContourIndex] = true;
					auto pairContour = contours->begin() + pairContourIndex;

					vector<Point2f> input;
					input.insert(input.end(), contour->begin(), contour->end());
					input.insert(input.end(), pairContour->begin(), pairContour->end());
					subdiv->insert(input);

					subdiv->getTriangleList(*triangleList);
					vector<Point> pt(3);
					for (size_t i = 0; i < triangleList->size(); i++)
					{
						Vec6f t = triangleList->at(i);
						pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
						pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
						pt[2] = Point(cvRound(t[4]), cvRound(t[5]));

						if ((isInRange<int>(pt[0].x, 0, source.cols) && isInRange<int>(pt[0].y, 0, source.rows))
							&& (isInRange<int>(pt[1].x, 0, source.cols) && isInRange<int>(pt[1].y, 0, source.rows))
							&& (isInRange<int>(pt[2].x, 0, source.cols) && isInRange<int>(pt[2].y, 0, source.rows))
							)
						{
							if (
								!(contoursIndex.at<uchar>(pt[0]) == contoursIndex.at<uchar>(pt[1])
									&& contoursIndex.at<uchar>(pt[1]) == contoursIndex.at<uchar>(pt[2])
									&& contoursIndex.at<uchar>(pt[0]) == contoursIndex.at<uchar>(pt[2]))

								)
							{
								resultTriangleList.push_back(t);
							}
						}
					}
				}
				else //ONE CONTOUR
				{
					vector<Point2f> input;
					input.insert(input.end(), contour->begin(), contour->end());
					subdiv->insert(input);

					subdiv->getTriangleList(*triangleList);
					vector<Point> pt(3);
					for (size_t i = 0; i < triangleList->size(); i++)
					{
						Vec6f t = triangleList->at(i);
						pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
						pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
						pt[2] = Point(cvRound(t[4]), cvRound(t[5]));

						if ((isInRange<int>(pt[0].x, 0, source.cols) && isInRange<int>(pt[0].y, 0, source.rows))
							&& (isInRange<int>(pt[1].x, 0, source.cols) && isInRange<int>(pt[1].y, 0, source.rows))
							&& (isInRange<int>(pt[2].x, 0, source.cols) && isInRange<int>(pt[2].y, 0, source.rows))
							)
						{
							if (
								isTraingleLess(pt[0], pt[1], pt[2], 1, kTrianglesFilter)
								)
							{
								resultTriangleList.push_back(t);
							}
						}

					}
				}
			}
			contoursTriangleList->push_back(resultTriangleList);
		}
	}
}

bool ImageParser::isTraingleLess(Point a, Point b, Point c, uchar threshold, float limit)
{
	vector<Point> vertex = { a,b,c };
	Rect traingRect = boundingRect(vertex);

	Mat mask = Mat::zeros(traingRect.size(), CV_8U);
	for (size_t i = 0; i < vertex.size(); i++)
	{
		vertex.at(i) = Point(vertex.at(i).x - traingRect.x, vertex.at(i).y - traingRect.y);
	}
	fillConvexPoly(mask, vertex, 255);
	int pointsCount = 0;
	int upperThresholdPoint = 0;

	for (size_t x = traingRect.x; x < traingRect.x + traingRect.width; x++)
	{
		for (size_t y = traingRect.y; y < traingRect.y + traingRect.height; y++)
		{
			Point check(x - traingRect.x, y - traingRect.y);
			if (isInRange(check.x, 0, source.cols) && isInRange(check.y, 0, source.rows))
			{
				if (mask.at<uchar>(check) == 255) {

					uchar value = source.at<uchar>(y, x);

					if (value > threshold)
					{
						upperThresholdPoint++;
					}
					pointsCount++;
				}
			}
		}
	}
	float rate = (float)upperThresholdPoint / pointsCount;
	return (rate < limit) ? true : false;
}

bool ImageParser::findPairContourIndex(int contourIndex, const Point & point, int & pairIndex, Point & result)
{
	Point pairPoint;
	pairIndex = -1;
	float angle = angles.at<float>(point);

	Mat disp(angles);

	if (angle != 0)
	{
		for (float z = 0; z < 45; z++) {
			for (int b = 0; b < 2; b++)
			{
				for (float l = 3.0f;
					isInRange(pairPoint.x = point.x - l * cosf((angle + (b ? z : -z)) * RAD), 0, angles.cols)
					&& isInRange(pairPoint.y = point.y - l * sinf((angle + (b ? z : -z)) * RAD), 0, angles.rows); l += 0.2f)
				{
					disp.at<float>(pairPoint) = 255;

					if (contoursIndex.at<uchar>(pairPoint) != 0) {
						pairIndex = contoursIndex.at<uchar>(pairPoint) - 1;
						result = pairPoint;
						break;
					}
					if (pairIndex != -1) break;
				}
				if (pairIndex != -1) break;
			}
		}
	}
	return ((pairIndex != -1) ? true : false);
}

void ImageParser::SetIndexMat() {
	for (int i = 0; i < contours->size() - 1; i++)
	{
		drawContours(contoursIndex, *contours, i, Scalar(i + 1), 2, 8, vector<Vec4i>(), 0, Point());
	}
}

void ImageParser::SetAngleMat() {
	int scale = 1;
	int delta = 0;
	int ddepth = CV_32F;

	Mat src_gray;
	source.copyTo(src_gray);

	Mat grad_x, grad_y;
	Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);

	phase(grad_x, grad_y, angles, true);

	for (int x = 0; x < angles.cols; x++)
	{
		for (int y = 0; y < angles.rows; y++)
		{
			if (angles.at<float>(y, x) == 0) 
			{
				if (grad_x.at<float>(y, x) != 0 || grad_y.at<float>(y, x) != 0)
					angles.at<float>(y, x) = 1;
			}
		}
	}
}

void ImageParser::findContours()
{
	Mat temp(source.size(), source.type());
	source.copyTo(temp);
	cv::findContours(temp, *contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE, Point(0, 0));
}


void ImageParser::removeBeeline(vector<Point> & contour, float offset) {
	Mat drawing(Size(512, 512), CV_8UC3);

	for (auto pointIt = contour.begin(); pointIt < contour.end(); pointIt++)
	{
		vector<Point>::iterator prevIt;
		if (pointIt == contour.begin())
			prevIt = (contour.end() - 1);
		else
			prevIt = pointIt - 1;

		if (decDistance(*pointIt, *prevIt) > offset) {
			vector<Point> additionalPoints;

			addPointsToMiddle(*prevIt, *pointIt, offset, additionalPoints);

			int oldIndex = pointIt - contour.begin();

			contour.reserve(contour.size() + additionalPoints.size());
			contour.insert(pointIt, additionalPoints.begin(), additionalPoints.end());
			pointIt = contour.begin() + oldIndex + additionalPoints.size();
		}
	}
}

void ImageParser::moreBlack(Mat & img, const int size) {
	for (int x = 0; x < img.rows; x++)
	{
		for (int y = 0; y < img.cols; y++)
		{
			if (img.at<uchar>(x, y) == 0)
			{
				for (int j = x - size / 2; j < x + size / 2; j++)
				{
					if (isInRange<int>(j, 0, img.rows))
					{
						for (int z = y - size / 2; z < y + size / 2; z++)
						{
							if (isInRange<int>(z, 0, img.cols))
							{
								img.at<uchar>(j, z) = 2;
							}
						}
					}
				}
			}
		}
	}
	threshold(img, img, 127, 255, THRESH_BINARY);
}

