/*
 * feature_extractor.h
 *
 *  Created on: Aug 27, 2014
 *      Author: gachiemchiep
 */

#ifndef FEATURE_EXTRACTOR_H_
#define FEATURE_EXTRACTOR_H_

#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <math.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>

const std::vector<std::string> methods { "SCD", "EHD", "CD", "ALL" };

/*
 * ehd feature descriptor's filter
 * vertical, horizontal, 45 degree, 135 degree, non-direction
 * edges's threshold
 */

const std::vector<float> ver_edge_filter { 1, -1, 1, -1 };
const std::vector<float> hor_edge_filter { 1, 1, -1, -1 };
const std::vector<float> dia45_edge_filter { std::sqrt(2), 0, 0, -std::sqrt(2) };
const std::vector<float> dia135_edge_filter { 0, std::sqrt(2), -std::sqrt(2), 0 };
const std::vector<float> nond_edge_filter { 2, -2, -2, 2 };
const std::vector<std::vector<float> > edge_filters { ver_edge_filter,
		hor_edge_filter, dia45_edge_filter, dia135_edge_filter, nond_edge_filter };
const float EDGE_THRES = 50;
const int ehd_block = 1024; // block number for each sub image, sqrt(1024)= 32
const int NORM = 4096;
/*
 * Maximum of vector
 */
template<typename num_type>
num_type maximum(std::vector<num_type>& nums) {
	num_type max_val = nums[0];
	for (int i = 0; i < nums.size(); i++) {
		if (max_val < nums[i]) {
			max_val = nums[i];
		}
	}
	return max_val;
}

/*
 * Print out vetor
 */
template<typename num_type>
void display(std::vector<num_type> const& nums) {
	for (int i = 0; i < nums.size(); i++) {
		std::cerr << nums[i] << " ";
	}
	std::cerr << "\n";
}

/*
 * Calculate edge streng
 */
template<typename num_type>
num_type multiply_scalars(std::vector<num_type> const& scalar1,
		std::vector<num_type> const& scalar2) {
	num_type result = 0;
	if (scalar1.size() != scalar2.size()) {
		std::cerr << "Input scalar should be equal \n";
		std::cerr << scalar1.size() << " " << scalar2.size() << "\n";
		return result;
	} else {
		for (int i = 0; i < scalar1.size(); i++) {
			result += scalar1[i] * scalar2[i];
		}
	}
	result = std::abs(result);
	return result;
}

class feature_extractor {
public:
	feature_extractor();
	virtual ~feature_extractor();
	void set_img_path(std::string img_path);
	std::string get_img_path();
	void set_method(std::string method);
	std::string get_method();
	cv::Mat get_feature();
	bool is_valid();
	cv::Mat calculate_feature();
private:
	std::string m_img;
	cv::Mat m_frame;
	std::string m_method;
	cv::Mat m_feature;
	bool check_img_path();
	bool check_method();
	cv::Mat calculate_scd(cv::Mat &frame);
	cv::Mat calculate_ehd(cv::Mat &frame);
	cv::Mat calculate_cd(cv::Mat &frame);
	// calculate edge streng for block
	std::vector<float> edges_streng(cv::Mat &frame_gray, cv::Rect block_rect);
	// divide rect into smaller part rect -> y*x smaller rect
	std::vector<cv::Rect> split_rect(cv::Rect rect, int y_parts_count,
			int x_parts_count);
	float strongest_edge(std::vector<float>& edges_streng, float threshold);
	bool is_skin_pixel(cv::Vec3b bgr);
};

#endif /* FEATURE_EXTRACTOR_H_ */
