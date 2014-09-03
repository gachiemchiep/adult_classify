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

//const int ADULT_TESTING_COUNT = 150;
//const int NON_ADULT_TESTING_COUNT = 1000;

const float ADULT_TEST_PER = 0.1;
const float NON_ADULT_TEST_PER = 0.1;
const float NN_PER = 0.1;

const int RANK_MAX_SCORE = 1000;
//const int NN_COUNT = 100; /**< nearest neighbor(base on rank score) count*/
const float THRES = 0.7; /**< Thres value to decide whether image is adult or non*/

using namespace std;
using namespace cv;

struct features_distances {
	float scd_distance;
	float ehd_distance;
	float cd_distance;
};

/*
 * position, distances
 */
struct pos_val {
	int pos;
	float val;
};

/*
 * position, rank score
 */

struct pos_rank {
	int pos;
	int rank;
};

bool compare_distances_ascending(pos_val a, pos_val b) {
	return a.val < b.val;
}

bool compare_ranks_descending(pos_rank a, pos_rank b) {
	return a.rank > b.rank;
}

template<typename num_type>
void display(std::vector<num_type>& vector_contains) {
	for (int i = 0; i < vector_contains.size(); i++) {
		std::cerr << vector_contains[i] << " ";
	}
	std::cerr << "\n";
}

// Divide all_vector into testing, learning set
template<typename num_type>
void split_vector(std::vector<num_type>& all_vector,
		std::vector<num_type>& testing_vector,
		std::vector<num_type>& learning_vector, int testing_count) {
	testing_vector.erase(testing_vector.begin(), testing_vector.end());
	learning_vector.erase(learning_vector.begin(), learning_vector.end());
	for (unsigned i = 0; i < all_vector.size(); i++) {
		if (i < testing_count) {
			testing_vector.push_back(all_vector[i]);
		} else {
			learning_vector.push_back(all_vector[i]);
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
features_distances calculate_distances(cv::Mat &feature1, cv::Mat &feature2) {

	features_distances result;

	std::vector<float> results;
	cv::Mat scd1 = feature1(cv::Rect(0, 0, 256, 1));
	cv::Mat scd2 = feature2(cv::Rect(0, 0, 256, 1));

	result.scd_distance = float(cv::norm(scd1, scd2, NORM_L2));

	cv::Mat ehd1_local = feature1(cv::Rect(256, 0, 80, 1));
	cv::Mat ehd2_local = feature2(cv::Rect(256, 0, 80, 1));

	float ehd_local_dist = float(cv::norm(ehd1_local, ehd2_local, NORM_L2));

	cv::Mat ehd1_global = feature1(cv::Rect(336, 0, 5, 1));
	cv::Mat ehd2_global = feature2(cv::Rect(336, 0, 5, 1));

	float ehd_global_dist = float(cv::norm(ehd1_global, ehd2_global, NORM_L2));

	result.ehd_distance = ehd_local_dist + 5 * ehd_global_dist;

	cv::Mat cd1 = feature1(cv::Rect(341, 0, 21, 1));
	cv::Mat cd2 = feature2(cv::Rect(341, 0, 21, 1));

	result.cd_distance = float(cv::norm(cd1, cd2, NORM_L2));

	return result;
}

void parse_features(std::string features_file, std::vector<std::string> &names,
		std::vector<cv::Mat> &features);

/*
 * Calculate rank score base on distance
 * Smaller distance got higher rank score
 * Maximum score = max_rank
 */
cv::Mat calculate_rank(cv::Mat distances, int max_rank) {

	cv::Mat rank = cv::Mat::zeros(distances.rows, distances.cols, CV_16U);
	if (distances.rows != 1) {
		std::cerr << "Input matrix should be 1xwidth \n";
	} else {
		// Init an unsorted vec (position/value)
		std::vector<pos_val> unsorted_vec;
		for (int i = 0; i < distances.cols; i++) {
			pos_val tmp_pos_val;
			tmp_pos_val.pos = i;
			tmp_pos_val.val = distances.at<float>(0, i);
			unsorted_vec.push_back(tmp_pos_val);
		}
		// Then sort it into ascending order
		std::sort(unsorted_vec.begin(), unsorted_vec.end(),
				compare_distances_ascending);
		// Then making rank matrix
		for (int i = 0; i < max_rank; i++) {
			int score = max_rank - i;
			pos_val tmp_pos_val = unsorted_vec[i];
			rank.at<unsigned short>(0, tmp_pos_val.pos) = score;
		}
	}

	return rank;

}

/*
 * Find max_rank positions which have largest rank score value
 */
std::vector<pos_rank> find_large_rank(cv::Mat rank_mat, int max_count) {

	std::vector<pos_rank> result;

	if (rank_mat.rows != 1) {
		std::cerr
				<< "Input rank_mat for find_similar_pos should have height=1 \n";
	} else {
		// Init an unsorted vec (position/value)
		std::vector<pos_rank> unsorted_vec;
		for (int i = 0; i < rank_mat.cols; i++) {
			pos_rank tmp_pos_rank;
			tmp_pos_rank.pos = i;
			tmp_pos_rank.rank = rank_mat.at<unsigned short>(0, i);
			unsorted_vec.push_back(tmp_pos_rank);
		}
		// Then sort it into descending order
		std::sort(unsorted_vec.begin(), unsorted_vec.end(),
				compare_ranks_descending);
		// Then push max_count pos_val to result
		for (int i = 0; i < max_count; i++) {
			result.push_back(unsorted_vec[i]);
		}
	}

	return result;
}

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

	printf("Adult count: %d\n", adult_files.size());
	printf("non_adult count: %d\n", non_adult_files.size());

	// now shuffle and divide into testing, learning set
	std::vector<std::string> adult_files_learn, non_adult_files_learn,
			adult_files_test, non_adult_files_test;
	std::vector<cv::Mat> adult_features_learn, non_adult_features_learn,
			adult_features_test, non_adult_features_test;

	// TODO:
	// consider everything is fine
	// oop, files and feature

	int adult_test_count = floor(ADULT_TEST_PER * adult_files.size());
	int non_adult_test_count = floor(ADULT_TEST_PER * non_adult_files.size());

	split_vector(adult_files, adult_files_test, adult_files_learn,
			adult_test_count);
	split_vector(adult_features, adult_features_test, adult_features_learn,
			adult_test_count);
	split_vector(non_adult_files, non_adult_files_test, non_adult_files_learn,
			non_adult_test_count);
	split_vector(non_adult_features, non_adult_features_test,
			non_adult_features_learn, non_adult_test_count);

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

	std::cerr << "Testing:" << testing_features.size() << " adult="
			<< adult_features_test.size() << " non_adult="
			<< non_adult_features_test.size() << "\n";
	// merge into learning vector : ADULT_LEARNING_COUNT(adult) + NON_ADULT_LEARNING_COUNT
	std::vector<std::string> learning_files;
	learning_files.reserve(
			adult_files_learn.size() + non_adult_files_learn.size());
	learning_files.insert(learning_files.end(), adult_files_learn.begin(),
			adult_files_learn.end());
	learning_files.insert(learning_files.end(), non_adult_files_learn.begin(),
			non_adult_files_learn.end());

	std::vector<cv::Mat> learning_features;
	learning_features.reserve(
			adult_features_learn.size() + non_adult_features_learn.size());
	learning_features.insert(learning_features.end(),
			adult_features_learn.begin(), adult_features_learn.end());
	learning_features.insert(learning_features.end(),
			non_adult_features_learn.begin(), non_adult_features_learn.end());

	std::cerr << "Learning:" << learning_features.size() << " adult="
			<< adult_features_learn.size() << " non_adult="
			<< non_adult_features_learn.size() << "\n";

	// After merging shuffle features vecotr
	std::random_shuffle(adult_features_test.begin(), adult_features_test.end());
	std::random_shuffle(non_adult_features_test.begin(),
			non_adult_features_test.end());

	// calculate all distance and push into distances matrix
	// 3 distances matrix : scd, ehd, cd
	int distances_matrix_rows = testing_features.size();
	int distances_matrix_cols = learning_features.size();
	cv::Mat scd_distances = cv::Mat::zeros(distances_matrix_rows,
			distances_matrix_cols, CV_32F);
	cv::Mat ehd_distances = cv::Mat::zeros(distances_matrix_rows,
			distances_matrix_cols, CV_32F);
	cv::Mat cd_distances = cv::Mat::zeros(distances_matrix_rows,
			distances_matrix_cols, CV_32F);

	// distances with adult image
	// result is put in (h,w) h=[0, .., testing_features.size())
	//						  w=[0, .., learning_features.size())
	for (unsigned i = 0; i < testing_features.size(); i++) {
		cv::Mat feature1 = testing_features[i];
		for (unsigned j = 0; j < learning_features.size(); j++) {
			cv::Mat feature2 = learning_features[j];
			features_distances dists = calculate_distances(feature1, feature2);
			scd_distances.at<float>(i, j) = dists.scd_distance;
			ehd_distances.at<float>(i, j) = dists.ehd_distance;
			cd_distances.at<float>(i, j) = dists.cd_distance;
		}
	}

	cv::FileStorage save_scd_distances("scd_distances.xml",
			cv::FileStorage::WRITE);
	save_scd_distances << "scd" << scd_distances;
	save_scd_distances.release();

	cv::FileStorage save_ehd_distances("ehd_distances.xml",
			cv::FileStorage::WRITE);
	save_ehd_distances << "ehd" << ehd_distances;
	save_ehd_distances.release();

	cv::FileStorage save_cd_distances("cd_distances.xml",
			cv::FileStorage::WRITE);
	save_cd_distances << "cd" << cd_distances;
	save_cd_distances.release();

	// TODO calculate rank score base on distances matrix
	// change quantize size (to remove noise )

	// Rank score matrix
	int rank_matrix_rows = testing_features.size();
	int rank_matrix_cols = learning_features.size();
	cv::Mat scd_ranks = cv::Mat::zeros(rank_matrix_rows, rank_matrix_cols,
	CV_16U); /**< unsigned short*/
	cv::Mat ehd_ranks = cv::Mat::zeros(rank_matrix_rows, rank_matrix_cols,
	CV_16U);
	cv::Mat cd_ranks = cv::Mat::zeros(rank_matrix_rows, rank_matrix_cols,
	CV_16U);
	cv::Mat final_ranks = cv::Mat::zeros(rank_matrix_rows, rank_matrix_cols,
	CV_16U);

	// Calculate rank score for each matrix
	for (unsigned i = 0; i < testing_features.size(); i++) {
		cv::Mat scd_rank_i = calculate_rank(scd_distances.row(i),
				RANK_MAX_SCORE);
		scd_rank_i.copyTo(scd_ranks.row(i));
		cv::Mat ehd_rank_i = calculate_rank(ehd_distances.row(i),
				RANK_MAX_SCORE);
		ehd_rank_i.copyTo(ehd_ranks.row(i));
		cv::Mat cd_rank_i = calculate_rank(cd_distances.row(i), RANK_MAX_SCORE);
		cd_rank_i.copyTo(cd_ranks.row(i));
	}

	final_ranks = scd_ranks + ehd_ranks + cd_ranks;

	// Save rank matrix
//	cv::FileStorage save_scd_rank("scd_rank.xml", cv::FileStorage::WRITE);
//	save_scd_rank << "scd_rank" << scd_ranks;
//	save_scd_rank.release();
//
//	cv::FileStorage save_ehd_rank("ehd_rank.xml", cv::FileStorage::WRITE);
//	save_ehd_rank << "ehd_rank" << ehd_ranks;
//	save_ehd_rank.release();
//
//	cv::FileStorage save_cd_rank("cd_rank.xml", cv::FileStorage::WRITE);
//	save_cd_rank << "cd_rank" << cd_ranks;
//	save_cd_rank.release();
//
//	cv::FileStorage save_rank("rank.xml", cv::FileStorage::WRITE);
//	save_rank << "rank" << final_ranks;
//	save_rank.release();

	int nn_count = floor(NN_PER * learning_features.size());
	std::cerr << "Nearest Neighbord: " << nn_count << "\n";
	/**< Count corresponding adult, non-adult for each testing image*/
	// adult count - non_adult count
	cv::Mat testing_result = cv::Mat::zeros(testing_files.size(), 2, CV_16U);
	int adult_files_count = adult_files_learn.size();
	for (unsigned i = 0; i < testing_files.size(); i++) {
		std::vector<pos_rank> tmp_result = find_large_rank(final_ranks.row(i),
				nn_count);
//		std::cerr << format(final_ranks.row(0), "python") << "\n";
		for (unsigned j = 0; j < tmp_result.size(); j++) {
//			std::cerr <<"pos:" << tmp_result[i].pos << "-" << "rank:" << tmp_result[i].rank << "-";
			if (tmp_result[j].pos < adult_files_count) {
//				std::cerr << "adult" << ", ";
				testing_result.at<unsigned short>(i, 0) += 1;
			} else {
//				std::cerr << "non_adult" << ", ";
				testing_result.at<unsigned short>(i, 1) += 1;
			}
		}
	}
//	std::cerr << "Adult file result \n";
//	std::cerr << testing_result.rowRange(0, adult_files_test.size() - 1)
//			<< "\n";
//	std::cerr << "Non_adult file result \n";
//	std::cerr
//			<< testing_result.rowRange(adult_files_test.size(),
//					testing_files.size()) << "\n";

	std::cerr << "Classification's accuracy:\n";
	int adult_true_count = 0;
	int adult_false_count = 0;
	int non_adult_true_count = 0;
	int non_adult_false_count = 0;

	for (int i = 0; i < testing_result.rows; i++) {
		// adult file
		if (i < adult_test_count) {
			float per = float(testing_result.at<unsigned short>(i, 0))
					/ nn_count;
			if (per > THRES) {
				adult_true_count++;
			} else {
				adult_false_count++;
			}
		} else { // non_adult file
			float per = float(testing_result.at<unsigned short>(i, 0))
					/ nn_count;
			if (per < THRES) {
				non_adult_true_count++;
			} else {
				non_adult_false_count++;
			}
		}
	}

	int adult_accuracy = floor(float(adult_true_count)
			/ (adult_true_count + adult_false_count) * 100);
	int non_adult_accuracy = floor(float(non_adult_true_count)
			/ (non_adult_true_count + non_adult_false_count) * 100);

	std::cerr << "Adult:\t\t" << "true_count:" << adult_true_count
			<< " false_count:" << adult_false_count << " Per:" << adult_accuracy
			<< "%\n";
	std::cerr << "Non_Adult:\t" << "true_count:" << non_adult_true_count
			<< " false_count:" << non_adult_false_count << " Per:"
			<< non_adult_accuracy << "%\n";
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
