/*
 * feature_extractor.h
 *
 *  Created on: Aug 27, 2014
 *      Author: gachiemchiep
 */
/**
 * @class feature_extractor
 */


#ifndef FEATURE_EXTRACTOR_H_
#define FEATURE_EXTRACTOR_H_

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <math.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>
#include "params.h"

/*
 * ehd feature descriptor's filter
 * vertical, horizontal, 45 degree, 135 degree, non-direction
 * edges's threshold
 */

/*
 * TODO : move to utils.h
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
 * TODO move to utils.h
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
 * TODO move to utils.h
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
	void calculate_feature();
	void save_result(std::string result_file);

private:
	std::string m_img; /**< Input image location*/
	cv::Mat m_frame; /**< Input image matrix*/
	std::string m_method; /**< Input method*/
	cv::Mat m_feature; /**< Output feature matrix*/
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
	void save_feature(cv::Mat &feature, std::string file_name, std::string result_file);
};

#endif /* FEATURE_EXTRACTOR_H_ */
