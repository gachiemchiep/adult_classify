//============================================================================
// Name        : main.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {

	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << "image location \n";
		return -1;
	}
	cv::Mat m_face = cv::imread(argv[1], 1);
	cv::cvtColor(m_face, m_face, CV_BGR2HSV);

	cv::Mat m_skin;
	cv::inRange(m_face, cv::Scalar(6, 58, 0), cv::Scalar(38, 173, 255), m_skin);

//	cv::imshow("face", m_face);
	cv::imshow("face's skin", m_skin);

	cv::Mat m_kernel = cv::Mat::ones(5, 5, CV_8UC1);
	cv::Mat m_skin_smth;
	cv::Mat m_element = cv::getStructuringElement(MORPH_ELLIPSE, Size(5, 5),
			Point(1, 1));
	// erode(dilate)
	cv::morphologyEx(m_skin, m_skin_smth, MORPH_CLOSE, m_element);
	cv::blur(m_skin_smth, m_skin_smth, Size(3, 3), Point(1, 1));

	double d_min, d_max;
	cv::minMaxLoc(m_skin_smth, &d_min, &d_max);
	cv::threshold(m_skin_smth, m_skin_smth, d_max * 3 / 4, d_max,
			THRESH_BINARY);

	std::vector<std::vector<cv::Point> > v_contours;
	std::vector<cv::Vec4i> v_hierarchy;

	cv::findContours(m_skin_smth, v_contours, v_hierarchy, CV_RETR_EXTERNAL,
			CV_CHAIN_APPROX_SIMPLE, Point(1, 1));

	std::vector<std::vector<cv::Point> > v_contours_poly(v_contours.size());
	std::vector<cv::Rect> v_rect(v_contours.size());
	std::vector<cv::Point2f> v_centers(v_contours.size());
	std::vector<float> v_radius(v_contours.size());

	for (unsigned i = 0; i < v_contours.size(); i++) {
		cv::approxPolyDP( cv::Mat(v_contours[i]), v_contours_poly[i], 3, true);
		v_rect[i] = cv::boundingRect(cv::Mat(v_contours_poly[i]));
//		cv::minEnclosingCircle((Mat)v_contours_poly[i], v_centers[i], v_radius[i]);
	}

	cv::Mat m_bg_rm = cv::Mat::zeros(m_face.rows, m_face.cols, m_face.type());
	cv::Mat m_test = cv::Mat::zeros(m_face.rows, m_face.cols, m_face.type());
	cv::cvtColor(m_face, m_face, CV_HSV2BGR);
	for (unsigned i = 0; i < v_contours.size(); i++) {
//		cv::rectangle(m_test, v_rect[i].tl(), v_rect[i].br(), Scalar(128, 128, 128), 1, 8, 0);
		for (int h = v_rect[i].tl().y; h < v_rect[i].br().y; h++) {
			for (int w = v_rect[i].tl().x; w < v_rect[i].br().x; w++) {
				m_bg_rm.at<Vec3b>(h, w) = m_face.at<Vec3b>(h, w);
				}
		}

	}
//	imshow("m_test", m_test);
	imshow("background remove", m_bg_rm);

	cv::waitKey(0);

	return 0;
}

