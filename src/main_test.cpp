/*
 * main.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: gachiemchiep
 *      Description : main
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include "feature_extractor.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>

const int ADULT_TESTING_COUNT = 250;
const int NON_ADULT_TESTING_COUNT = 100;

using namespace std;
using namespace cv;

template<typename num_type>
void display(std::vector<num_type>& vector_contains) {
	for (int i = 0; i < vector_contains.size(); i++) {
		std::cerr << vector_contains[i] << " ";
	}
	std::cerr << "\n";
}

// shuffle then divide vector into testing, learning set
template<typename num_type>
void split_vector(std::vector<num_type>& all_vector,
		std::vector<num_type>& testing_vector,
		std::vector<num_type>& learning_vector, int testing_count) {
	std::vector<int> shuffle;
	for (unsigned i = 0; i < all_vector.size(); i++) {
		shuffle.push_back(i);
	}
	std::random_shuffle(shuffle.begin(), shuffle.end());
	for (unsigned i = 0; i < all_vector.size(); i++) {
		if (i < testing_count) {
			testing_vector.push_back(all_vector[shuffle[i]]);
		} else {
			learning_vector.push_back(all_vector[shuffle[i]]);
		}
	}
}

// convert a vector of string to mat
cv::Mat convert_to_mat(std::vector<string> nums_vector) {
	cv::Mat result = cv::Mat::zeros(1, nums_vector.size(), CV_16U);
	for (unsigned i = 0; i < nums_vector.size(); i++) {
		result.at<unsigned short>(0, i) = boost::lexical_cast<unsigned short>(
				nums_vector[i]);
	}

	return result;
}

// calculate scd, ehd, cd distances 1x362
std::vector<float> calculate_distances(cv::Mat &feature1, cv::Mat &feature2) {

	std::vector<float> results;
	cv::Mat scd1 = feature1(cv::Rect(0, 0, 256, 1));
	cv::Mat scd2 = feature2(cv::Rect(0, 0, 256, 1));
//	std::cerr << scd1 << "\n";
//	std::cerr << scd2 << "\n";
	float scd_dist = float(cv::norm(scd1, scd2, NORM_L2));
	results.push_back(scd_dist);

	cv::Mat ehd1_local = feature1(cv::Rect(256, 0, 80, 1));
	cv::Mat ehd2_local = feature2(cv::Rect(256, 0, 80, 1));
//	std::cerr << ehd1_local << "\n";
//	std::cerr << ehd2_local << "\n";
	float ehd_local_dist = float(cv::norm(ehd1_local, ehd2_local, NORM_L2));

	cv::Mat ehd1_global = feature1(cv::Rect(336, 0, 5, 1));
	cv::Mat ehd2_global = feature2(cv::Rect(336, 0, 5, 1));
//	std::cerr << ehd1_global << "\n";
//	std::cerr << ehd2_global << "\n";
	float ehd_global_dist = float(cv::norm(ehd1_global, ehd2_global, NORM_L2));
	float ehd_dist = ehd_local_dist + 5 * ehd_global_dist;
	results.push_back(ehd_dist);

	cv::Mat cd1 = feature1(cv::Rect(341, 0, 21, 1));
	cv::Mat cd2 = feature2(cv::Rect(341, 0, 21, 1));
//	std::cerr << cd1 << "\n";
//	std::cerr << cd2 << "\n";
	float cd_dist = float(cv::norm(cd1, cd2, NORM_L2));
	results.push_back(cd_dist);

	return results;
}

void parse_features(std::string features_file, std::vector<std::string> &names,
		std::vector<cv::Mat> &features);

int main(int argc, char *argv[]) {

	std::cerr << "Beginning testing phase \n";

	if (argc != 3) {
		std::cout << "Usage: " << argv[0]
				<< " adult_feature.txt non_adult_feature.txt \n";
		return -1;
	}

	// TODO get std::vector<string> filenames
	// and corresponding matrix from file

	std::string adult_features_file = std::string(argv[1]);
	std::string non_adult_features_file = std::string(argv[2]);

	std::vector<std::string> adult_files, non_adult_files;
	std::vector<cv::Mat> adult_features, non_adult_features;

	parse_features(adult_features_file, adult_files, adult_features);
	parse_features(non_adult_features_file, non_adult_files,
			non_adult_features);

	printf("Aadult count: %d\n", adult_files.size());
	printf("non_adult count: %d\n", non_adult_files.size());
	std::cerr << adult_files.size() << "\n";
	std::cerr << non_adult_files.size() << "\n";

	// now shuffle and divide into testing, learning set
	std::vector<std::string> adult_files_learn, non_adult_files_learn,
			adult_files_test, non_adult_files_test;
	std::vector<cv::Mat> adult_features_learn, non_adult_features_learn,
			adult_features_test, non_adult_features_test;

	// TODO:
	// consider everything is fine
	// try , catch will be added later

	split_vector(adult_files, adult_files_test, adult_files_learn,
			ADULT_TESTING_COUNT);
	split_vector(adult_features, adult_features_test, adult_features_learn,
			ADULT_TESTING_COUNT);
	split_vector(non_adult_files, non_adult_files_test, non_adult_files_learn,
			NON_ADULT_TESTING_COUNT);
	split_vector(non_adult_features, non_adult_features_test,
			non_adult_features_learn, NON_ADULT_TESTING_COUNT);

	printf("adult files: all: %d, test: %d, learn: %d \n", adult_files.size(),
			adult_files_test.size(), adult_files_learn.size());
	printf("adult features: all: %d, test: %d, learn: %d \n",
			adult_features.size(), adult_features_test.size(),
			adult_features_learn.size());
	printf("non_adult files: all: %d, test: %d, learn: %d \n",
			non_adult_files.size(), non_adult_files_test.size(),
			non_adult_files_learn.size());
	printf("non_adult features: all: %d, test: %d, learn: %d \n",
			non_adult_features.size(), non_adult_features_test.size(),
			non_adult_features_learn.size());

	// TODO: now the hard part , calculate rank

	// merge into testing vector : ADULT_TESTING_COUNT(adult) + NON_ADULT_TESTING_COUNT
	std::vector<std::string> testing_files;
	testing_files.reserve(
			adult_files_test.size() + non_adult_files_test.size());
	testing_files.insert(testing_files.end(), adult_files_test.begin(),
			adult_files_test.end());
	testing_files.insert(testing_files.end(), non_adult_files_test.begin(),
			non_adult_files_test.end());

	std::vector<cv::Mat> testing_features;
	testing_features.reserve(
			adult_features_test.size() + non_adult_features_test.size());
	testing_features.insert(testing_features.end(), adult_features_test.begin(),
			adult_features_test.end());
	testing_features.insert(testing_features.end(),
			non_adult_features_test.begin(), non_adult_features_test.end());

	std::cerr << testing_files.size() << "\n";
	std::cerr << testing_features.size() << "\n";

	// calculate all distance and push into distances matrix
	// size : testing count * 3*(adult_count + non_adult_count)
	// 3 type of distances
	int distances_matrix_rows = testing_features.size();
	int distances_matrix_cols = 3
			* (adult_features_learn.size() + non_adult_features_learn.size());
	cv::Mat distances_matrix = cv::Mat::zeros(distances_matrix_rows,
			distances_matrix_cols, CV_32F);

	// distances with adult image
	// result is put in (i,j) i=[0, .., testing_features.size())
	//						  j=[0, .., 3*adult_features_learn.size())
	for (unsigned i = 0; i < testing_features.size(); i++) {
		cv::Mat feature1 = testing_features[i];
		for (unsigned j = 0; j < adult_features_learn.size(); j++) {
			cv::Mat feature2 = adult_features_learn[j];
			std::vector<float> dists = calculate_distances(feature1, feature2);
			for (int k = 0; k < 3; k++) {
				int pos = 3 * j + k;
				distances_matrix.at<float>(i, pos) = dists[k];
			}
		}
	}

	// distances with non_adult image
	// result is put in (i,j) i=[0, .., testing_features.size())
	//						  j=[3*adult_features_leanr.size() + 1, .., 3*non_adult_features_learn.size())
	int shift_range = 3 * adult_features_learn.size();
	for (unsigned i = 0; i < testing_features.size(); i++) {
		cv::Mat feature1 = testing_features[i];
		for (unsigned j = 0; j < non_adult_features_learn.size(); j++) {
			cv::Mat feature2 = non_adult_features_learn[j];
			std::vector<float> dists = calculate_distances(feature1, feature2);
			for (int k = 0; k < 3; k++) {
				int pos = 3 * j + k + shift_range;
				distances_matrix.at<float>(i, pos) = dists[k];
			}
		}
	}

	cv::FileStorage write_distances_matrix("distances_matrix.xml", cv::FileStorage::WRITE);
	write_distances_matrix << "Distances" << distances_matrix;
	write_distances_matrix.release();

	// TODO calculate rank score base on distances matrix
	// change quantize size (to remove noise )

	return 0;
}

void parse_features(std::string features_file, std::vector<std::string> &names,
		std::vector<cv::Mat> &features) {
	std::string line;
	std::ifstream read_file(features_file);
	std::vector<std::string> line_contains;
	if (read_file.is_open()) {
		while (std::getline(read_file, line)) {
			// split
			boost::split(line_contains, line, boost::is_any_of(","));
			// first element is file name
			std::string file_name = line_contains[0];
			line_contains.erase(line_contains.begin());
			// the others is feature vector
			cv::Mat file_feature = convert_to_mat(line_contains);

			names.push_back(file_name);
			features.push_back(file_feature);
		}
	} else {
		std::cerr << "Unable to open file !!!\n";
	}
}
