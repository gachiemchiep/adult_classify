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

const std::vector<std::string> methods {"SCD", "EHD", "CD", "ALL"};

/*
 * ehd feature descriptor's filter
 * vertical, horizontal, 45 degree, 135 degree, non-direction
 * edges's threshold
 */
const float ver_edge_filter[2][2] = {{1, -1}, {1, -1}};
const float hor_edge_filter[2][2] = {{1, 1}, {-1, -1}};
const float dia45_edge_filter[2][2] = {{std::sqrt(2), 0},{0, -std::sqrt(2)}};
const float dia135_edge_filter[2][2] = {{0, std::sqrt(2)}, {-std::sqrt(2), 0}};
const float nond_edge_filter[2][2] = {{2, -2}, {-2, 2}};
const float edge_thres = 0.2;
const int ehd_block = 1024; // block number for each sub image, sqrt(1024) = 32
/*
 *
 */
template<typename num_type>
num_type maximum(std::vector<num_type> const& nums) {
	typename std::vector<num_type>::iterator it_nums;
	num_type max_val = &nums.begin();
	for (it_nums = nums.begin(); it_nums != nums.end();++it_nums ) {
		if (max_val < &it_nums) {
			max_val = &it_nums;
		}
	}
	return max_val;
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
};

#endif /* FEATURE_EXTRACTOR_H_ */
