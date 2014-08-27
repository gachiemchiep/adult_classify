/*
 * feature_extractor.cpp
 *
 *  Created on: Aug 27, 2014
 *      Author: gachiemchiep
 */

#include "feature_extractor.h"

feature_extractor::feature_extractor() {
	// TODO Auto-generated constructor stub
}

feature_extractor::~feature_extractor() {
	// TODO Auto-generated destructor stub
}

void feature_extractor::set_img_path(std::string img_path) {
	m_img = img_path;
}

std::string feature_extractor::get_img_path() {
	return m_img;
}

void feature_extractor::set_method(std::string method) {
	m_method = method;
}

std::string feature_extractor::get_method() {
	return m_method;
}

cv::Mat feature_extractor::get_feature() {
	return m_feature;
}

bool feature_extractor::check_img_path() {
	cv::Mat tmp = cv::imread(m_img, 1);
	if (tmp.data != NULL) {
		return true;
	} else {
		return false;
	}
}

bool feature_extractor::check_method() {
	if (std::find(methods.begin(), methods.end(), m_method) == methods.end()) {
		std::cerr << m_method << " is not valid. \n";
		std::cerr << 'Valid method "SCD", "EHD", "CD", "ALL"\n';
		std::cerr << "Should use ALL only \n";
		return false;
	} else {
		return true;
	}
}

bool feature_extractor::is_valid() {
	if ((check_img_path()) && (check_method())) {
		std::cerr << m_img << " " << m_method << " is usable \n";
		return true;
	} else {
		std::cerr << m_img << " Or " << m_method << " is not valid \n";
		return false;
	}
}

void feature_extractor::calculate_feature() {
	if (is_valid()) {
		// read m_img as bgr matrix
		m_frame = cv::imread(m_img, 1);
		// TODO feature calculationg
//		calculate_scd(m_frame);
		calculate_ehd(m_frame);
	}
}

/*
 * Calculate Scalable color descriptor
 * opencv hsv is [0-179],[0-255],[0-255] for 8bit image
 * Algorithm
 * scd = hsv - 16x4x4 = 256 dimension matrix
 * Count and assigne hsv value into corresponding bin
 * s[1], s[2], ...
 * s[1] =  hsv [1, 1, 1]
 * s[4] = hsv [1, 1, 4]
 * s[5] = hsv [1, 2, 1]
 * s[256] = hsv [16, 4, 4]
 *
 */
cv::Mat feature_extractor::calculate_scd(cv::Mat &frame) {

	cv::Mat frame_hsv;
	cv::cvtColor(frame, frame_hsv, CV_BGR2HSV);
	cv::Mat scd = cv::Mat::zeros(1, 256, CV_32F);
	for (int h = 0; h < frame_hsv.rows; h++) {
		for (int w = 0; w < frame_hsv.cols; w++) {
			cv::Vec3b hsv = frame_hsv.at<cv::Vec3b>(h, w);
			int h_bin = hsv[0] / 16;
			int s_bin = hsv[1] / 16;
			int v_bin = hsv[2] / 16;
			int position = 16 * h_bin + 4 * s_bin + v_bin;
			scd.at<float>(0, position) += 1;
		}
	}
	// TODO normalize feature then quantitize
	scd = scd / frame.size().area();
	std::cerr << frame.size().area() << "\n";
	std::cerr << cv::sum(scd) << "\n";
	std::cerr << scd;
	return scd;
}

/*
 * Calculate ehd features descriptor
 * 16x5(local) + 1x5(global) = 85 dimension
 * Algorithm
 * image -> 4x4 sub_image
 * sub_image -> 1024 image_block
 * 	1024 = 32^2 , image_block_w = image_width / 32
 * 				  image_block_h = image_height / 32
 * image_block -> 2x2 sub_image_block (0, 1, 2, 3)
 * for each sub_image_block -> gray value -?
 * use equation 3-7 to calculate edge streng
 * compare with threshold to find edge type
 *
 * Read Dong's "Efficient use of local edge histogram" for more detail
 * To make it easier to calculate
 * resize image so that width and height chia het cho 8
 *
 */
cv::Mat feature_extractor::calculate_ehd(cv::Mat &frame) {
	cv::Mat ehd = cv::Mat::zeros(1, 85, CV_32F);
	int rate = std::sqrt(ehd_block);
	// image -> 4x4 sub_image
	// resize
	int img_h = frame.rows;
	int img_w = frame.cols;
	int new_img_h = (img_h / rate + 1) * rate;
	int new_img_w = (img_w / rate + 1) * rate;
	cv::Mat new_frame;
	cv::resize(frame, new_frame, cv::Size(new_img_w, new_img_h));

	std::vector<cv::Mat> sub_frames;
	int sub_img_h = new_img_h / 4;
	int sub_img_w = new_img_w / 4;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			int sub_img_rect_x = x * sub_img_w;
			int sub_img_rect_y = y * sub_img_h;
			cv::Rect sub_img_rect = cv::Rect(sub_img_rect_x, sub_img_rect_y,
					sub_img_w, sub_img_h);
			sub_frames.push_back(new_frame(sub_img_rect));
			cv::rectangle(new_frame, sub_img_rect.tl(), sub_img_rect.br(),
					cv::Scalar(0, 0, 255), 1, 8, 0);
		}
	}

	int img_block_w = new_img_w / rate;
	int img_block_h = new_img_h / rate;

	for (int y = 0; y < rate; y++) {
		for (int x = 0; x < rate; x++) {
			int img_block_rect_x = x * img_block_w;
			int img_block_rect_y = y * img_block_h;
			cv::Rect sub_img_rect = cv::Rect(img_block_rect_x, img_block_rect_y,
					img_block_w, img_block_h);
			cv::rectangle(new_frame, sub_img_rect.tl(), sub_img_rect.br(),
					cv::Scalar(0, 255, 0), 1, 8, 0);
		}
	}
//	for (int i = 0; i < sub_frames.size(); i++) {
//		cv::Mat sub_frame = sub_frames[i];
////		printf("%d %d \n", sub_frame.cols, sub_frame.rows);
//	}
	// TODO divide into img_block is ok now
	// let extract feature descriptor
	imshow("divide image", new_frame);
	return ehd;
}
//cv::Mat feature_extractor::calculate_cd(cv::Mat &frame);
