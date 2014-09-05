/*
 * execute_testing.h
 *
 *  Created on: Sep 5, 2014
 *      Author: gachiemchiep
 */

#ifndef EXECUTE_TESTING_H_
#define EXECUTE_TESTING_H_

#include <iostream>
#include <fstream>
#include <stdio.h>
#include "feature_extractor.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include "utils.h"

using namespace std;
using namespace cv;

class execute_testing {
public:
	execute_testing();
	virtual ~execute_testing();
	void set_adult_features_file(std::string adult_features_file);
	std::string get_adult_features_file();
	void set_non_adult_features_file(std::string non_adult_features_file);
	std::string get_non_adult_features_file();
	void set_method(std::string method_name);
	std::string get_method();
	void evaluate_result();
private:
	std::string m_adult_features_file;
	std::string m_non_adult_features_file;
	std::string m_method;
	cv::Mat m_distance;
	matrixes m_distances_matrixes;
	matrixes m_ranks_matrixes;

	std::vector<cv::Mat> m_adult_features, m_non_adult_features;
	std::vector<cv::Mat> m_test_features, m_learn_features;

	void get_data();
	void shuffle_data();
	void split_features();
	void calculate_distances();
	void calculate_ranks();
	void print_result();

	void parse_features(std::string features_file, std::vector<std::string> &names,
			std::vector<cv::Mat> &features);
	cv::Mat vector_to_mat(std::vector<std::string> nums_vector);
	features_distances feature_distances(cv::Mat feature1, cv::Mat feature2);
	cv::Mat rank_from_distances(cv::Mat distances, int max_rank);
	std::vector<nums_pair<int>> find_large_rank(cv::Mat rank_mat, int max_count);
};

#endif /* EXECUTE_TESTING_H_ */
