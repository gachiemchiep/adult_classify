/*
 * execute_testing.cpp
 *
 *  Created on: Sep 5, 2014
 *      Author: gachiemchiep
 */

#include "execute_testing.h"

execute_testing::execute_testing() {
	// TODO Auto-generated constructor stub

}

execute_testing::~execute_testing() {
	// TODO Auto-generated destructor stub
}

/**
 * Set adult data's file location
 * Each line of data's file has the following structure
 * file_location,x[1],..x[256],x[257],..,x[341],x[342],..,x[366]
 */
void execute_testing::set_adult_features_file(std::string adult_features_file) {
	// TODO add check_valid
	m_adult_features_file = adult_features_file;
}

/**
 * Get adult data's file location
 */
std::string execute_testing::get_adult_features_file() {
	return m_adult_features_file;
}

/**
 * Set non-adult data's file location
 * Each line of data's file has the following structure
 * file_location,x[1],..x[256],x[257],..,x[341],x[342],..,x[366]
 */
void execute_testing::set_non_adult_features_file(
		std::string non_adult_features_file) {
	// TODO add check_valid
	m_non_adult_features_file = non_adult_features_file;
}

/**
 * Get non-adult data's file location
 */
std::string execute_testing::get_non_adult_features_file() {
	return m_non_adult_features_file;
}

/**
 * Set method
 * Corresponding to method's name these following features will
 * be used : scd, ehd, cd , all of scd-ehd-cd
 */
void execute_testing::set_method(std::string method_name) {
	m_method = method_name;
}

/**
 * Get method
 */
std::string execute_testing::get_method() {
	return m_method;
}

/**
 * Parse features from txt file into cv::Mat
 */
void execute_testing::parse_features(std::string features_file,
		std::vector<std::string> &names, std::vector<cv::Mat> &features) {
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
			cv::Mat file_feature = vector_to_mat(line_contains);

			names.push_back(file_name);
			features.push_back(file_feature);
		}
	} else {
		std::cerr << "Unable to open file !!!\n";
	}
}

/**
 * Convert vector of integer contain string into cv::Mat
 */
cv::Mat execute_testing::vector_to_mat(std::vector<std::string> nums_vector) {
	cv::Mat result = cv::Mat::zeros(1, nums_vector.size(), CV_16U);
	for (unsigned i = 0; i < nums_vector.size(); i++) {
		result.at<unsigned short>(0, i) = boost::lexical_cast<unsigned short>(
				nums_vector[i]);
	}
	return result;
}

/**
 * Parse adult data, non-adult data from corresponding
 * data file into process's flow
 */
void execute_testing::get_data() {
	// TODO check validity
	std::vector<std::string> adult_files, non_adult_files;

	parse_features(m_adult_features_file, adult_files, m_adult_features);
	parse_features(m_non_adult_features_file, non_adult_files,
			m_non_adult_features);
}

/**
 * Shuffle input data to get correct accuracy
 */
void execute_testing::shuffle_data() {
	std::random_shuffle(m_adult_features.begin(), m_adult_features.end());
	std::random_shuffle(m_non_adult_features.begin(),
			m_non_adult_features.end());
}

/**
 * Split adult, non-adult data into test and learn set.
 * The order is as follow
 * Adult_1
 * :
 * Adult_n
 * non-adult_1
 * :
 * non-adult_m
 */
void execute_testing::split_features() {

	// Shuffle data to get correct accuracy
	shuffle_data();

	std::vector<cv::Mat> adult_features_test, adult_features_learn;
	std::vector<cv::Mat> non_adult_features_test, non_adult_features_learn;

	int adult_test_count = floor(m_adult_features.size() * ADULT_TEST_PER);
	int non_adult_test_count = floor(
			m_non_adult_features.size() * NON_ADULT_TEST_PER);
	std::cerr << "adult_count:" << m_adult_features.size()
			<< "\nnon_adult_count:" << m_non_adult_features.size() << " \n";
	std::cerr << "adult_test_count:" << adult_test_count
			<< "\nnon_adult_test_count:" << non_adult_test_count << " \n";
	std::cerr << "adult_learn_count:"
			<< m_adult_features.size() - adult_test_count
			<< "\nnon_adult_learn_count:"
			<< m_non_adult_features.size() - non_adult_test_count << " \n";

	split_vector(m_adult_features, adult_features_test, adult_features_learn,
			adult_test_count);
	split_vector(m_non_adult_features, non_adult_features_test,
			non_adult_features_learn, non_adult_test_count);

	join_vector(adult_features_test, non_adult_features_test, m_test_features);
	join_vector(adult_features_learn, non_adult_features_learn,
			m_learn_features);

}

/**
 * Calculate all 3 types of distances and put into corresponding
 * contain matrix
 */
void execute_testing::calculate_distances() {
	int distances_matrix_rows = m_test_features.size();
	int distances_matrix_cols = m_learn_features.size();
	cv::Mat scd_distances = cv::Mat::zeros(distances_matrix_rows,
			distances_matrix_cols, CV_32F);
	cv::Mat ehd_distances = cv::Mat::zeros(distances_matrix_rows,
			distances_matrix_cols, CV_32F);
	cv::Mat cd_distances = cv::Mat::zeros(distances_matrix_rows,
			distances_matrix_cols, CV_32F);

	// distances with adult image
	// result is put in (h,w) h=[0, .., testing_features.size())
	//						  w=[0, .., learning_features.size())
	for (unsigned i = 0; i < m_test_features.size(); i++) {
		cv::Mat feature1 = m_test_features[i];
		for (unsigned j = 0; j < m_learn_features.size(); j++) {
			cv::Mat feature2 = m_learn_features[j];
			features_distances dists = feature_distances(feature1, feature2);
			scd_distances.at<float>(i, j) = dists.scd_distance;
			ehd_distances.at<float>(i, j) = dists.ehd_distance;
			cd_distances.at<float>(i, j) = dists.cd_distance;
		}
	}

	m_distances_matrixes.scd_matrix = scd_distances;
	m_distances_matrixes.ehd_matrix = ehd_distances;
	m_distances_matrixes.cd_matrix = cd_distances;
}

/**
 * 3 type of distances are calculated from parsed feature matrixes
 */
features_distances execute_testing::feature_distances(cv::Mat feature1,
		cv::Mat feature2) {
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

/**
 * From calculated distance matrixed, extract rank scores for each
 * feature. \n
 */
void execute_testing::calculate_ranks() {
	// Rank score matrix
	int rank_matrix_rows = m_test_features.size();
	int rank_matrix_cols = m_learn_features.size();
	cv::Mat scd_ranks = cv::Mat::zeros(rank_matrix_rows, rank_matrix_cols,
			CV_16U); /**< unsigned short*/
	cv::Mat ehd_ranks = cv::Mat::zeros(rank_matrix_rows, rank_matrix_cols,
			CV_16U);
	cv::Mat cd_ranks = cv::Mat::zeros(rank_matrix_rows, rank_matrix_cols,
			CV_16U);
	cv::Mat final_ranks = cv::Mat::zeros(rank_matrix_rows, rank_matrix_cols,
			CV_16U);

	// Calculate rank score for each matrix
	for (unsigned i = 0; i < m_test_features.size(); i++) {
		cv::Mat scd_rank_i = rank_from_distances(
				m_distances_matrixes.scd_matrix.row(i), RANK_MAX_SCORE);
		scd_rank_i.copyTo(scd_ranks.row(i));
		cv::Mat ehd_rank_i = rank_from_distances(
				m_distances_matrixes.ehd_matrix.row(i), RANK_MAX_SCORE);
		ehd_rank_i.copyTo(ehd_ranks.row(i));
		cv::Mat cd_rank_i = rank_from_distances(
				m_distances_matrixes.cd_matrix.row(i), RANK_MAX_SCORE);
		cd_rank_i.copyTo(cd_ranks.row(i));
	}

	m_ranks_matrixes.scd_matrix = scd_ranks;
	m_ranks_matrixes.ehd_matrix = ehd_ranks;
	m_ranks_matrixes.cd_matrix = cd_ranks;
}

/**
 * Rank is calculated as following formula : \n
 * n-th lowest distance feature's rank score
 * = number of nearest neighbor - n
 */
cv::Mat execute_testing::rank_from_distances(cv::Mat distances, int max_rank) {

	cv::Mat rank = cv::Mat::zeros(distances.rows, distances.cols, CV_16U);
	if (distances.rows != 1) {
		std::cerr << "Input matrix should be 1xwidth \n";
	} else {
		// Init an unsorted vec (position/value)
		std::vector<nums_pair<float> > unsorted_vec;
		for (int i = 0; i < distances.cols; i++) {
			nums_pair<float> tmp_pos_val;
			tmp_pos_val.position = i;
			tmp_pos_val.value = distances.at<float>(0, i);
			unsorted_vec.push_back(tmp_pos_val);
		}
		// Then sort it into ascending order
		std::sort(unsorted_vec.begin(), unsorted_vec.end(),
				compare_ascending<nums_pair<float>>);
		// Then making rank matrix
		for (int i = 0; i < max_rank; i++) {
			int score = max_rank - i;
			nums_pair<float> tmp_pos_val = unsorted_vec[i];
			rank.at<unsigned short>(0, tmp_pos_val.position) = score;
		}
	}

	return rank;
}

/**
 * Count accurary base on rank scores
 */
void execute_testing::print_result() {
	cv::Mat rank;

	if (m_method == "SCD") {
		std::cerr << "Method is SCD\n";
		rank = m_ranks_matrixes.scd_matrix;
	} else if (m_method == "EHD") {
		std::cerr << "Method is EHD\n";
		rank = m_ranks_matrixes.ehd_matrix;
	} else if (m_method == "CD") {
		std::cerr << "Method is CD\n";
		rank = m_ranks_matrixes.cd_matrix;
	} else {
		std::cerr << "Method is ALL\n";
		rank = m_ranks_matrixes.scd_matrix + m_ranks_matrixes.ehd_matrix
				+ m_ranks_matrixes.cd_matrix;
	}

	cv::FileStorage save_ranks("sum_ranks_f.xml", cv::FileStorage::WRITE);
	save_ranks << "cd" << rank;
	save_ranks.release();

	int nn_count = floor(NN_PER * m_learn_features.size());
	std::cerr << "Nearest Neighbord: " << nn_count << "\n";
	cv::Mat testing_result = cv::Mat::zeros(m_test_features.size(), 2, CV_16U);
	int adult_features_learn_count = m_adult_features.size()
			- floor(m_adult_features.size() * ADULT_TEST_PER);

	for (unsigned i = 0; i < m_test_features.size(); i++) {
		std::vector<nums_pair<int> > tmp_result = find_large_rank(rank.row(i),
				nn_count);
		for (unsigned j = 0; j < tmp_result.size(); j++) {
			if (tmp_result[j].position < adult_features_learn_count) {
				testing_result.at<unsigned short>(i, 0) += 1;
			} else {
				testing_result.at<unsigned short>(i, 1) += 1;
			}
		}
	}
	std::cerr << "Classification's accuracy:\n";
	int adult_true_count = 0;
	int adult_false_count = 0;
	int non_adult_true_count = 0;
	int non_adult_false_count = 0;
	int adult_features_test_count = floor(
			m_adult_features.size() * ADULT_TEST_PER);
	for (int i = 0; i < testing_result.rows; i++) {
		// adult file
		if (i < adult_features_test_count) {
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

	int adult_accuracy = floor(
			float(adult_true_count) / (adult_true_count + adult_false_count)
					* 100);
	int non_adult_accuracy = floor(
			float(non_adult_true_count)
					/ (non_adult_true_count + non_adult_false_count) * 100);

	std::cerr << "Adult:\t\t" << "true_count:" << adult_true_count
			<< " false_count:" << adult_false_count << " Per:" << adult_accuracy
			<< "%\n";
	std::cerr << "Non_Adult:\t" << "true_count:" << non_adult_true_count
			<< " false_count:" << non_adult_false_count << " Per:"
			<< non_adult_accuracy << "%\n";
}

/**
 * Find max_rank positions which have largest rank score value
 */
std::vector<nums_pair<int>> execute_testing::find_large_rank(cv::Mat rank_mat,
		int max_count) {

	std::vector<nums_pair<int>> result;

	if (rank_mat.rows != 1) {
		std::cerr
				<< "Input rank_mat for find_similar_pos should have height=1 \n";
	} else {
		// Init an unsorted vec (position/value)
		std::vector<nums_pair<int>> unsorted_vec;
		for (int i = 0; i < rank_mat.cols; i++) {
			nums_pair<int> tmp_pos_rank;
			tmp_pos_rank.position = i;
			tmp_pos_rank.value = rank_mat.at<unsigned short>(0, i);
			unsorted_vec.push_back(tmp_pos_rank);
		}
		// Then sort it into descending order
		std::sort(unsorted_vec.begin(), unsorted_vec.end(),
				compare_descending<nums_pair<int>>);
		// Then push max_count pos_val to result
		for (int i = 0; i < max_count; i++) {
			result.push_back(unsorted_vec[i]);
		}
	}

	return result;
}

/**
 * Do testing,
 * Print out result to std::cerr
 */
void execute_testing::evaluate_result() {
	// TODO check validity
	get_data(); // OK
	split_features();
	calculate_distances(); // distances is ok, nothing wrong
	calculate_ranks();
	print_result();

// Need result files. Uncomment these lines below
//	cv::FileStorage save_scd_distances("scd_distances_f.xml",
//			cv::FileStorage::WRITE);
//	save_scd_distances << "scd" << m_distances_matrixes.scd_matrix;
//	save_scd_distances.release();
//
//	cv::FileStorage save_ehd_distances("ehd_distances_f.xml",
//			cv::FileStorage::WRITE);
//	save_ehd_distances << "ehd" << m_distances_matrixes.ehd_matrix;
//	save_ehd_distances.release();
//
//	cv::FileStorage save_cd_distances("cd_distances_f.xml",
//			cv::FileStorage::WRITE);
//	save_cd_distances << "cd" << m_distances_matrixes.cd_matrix;
//	save_cd_distances.release();
//
//	cv::FileStorage save_scd_ranks("scd_ranks_f.xml", cv::FileStorage::WRITE);
//	save_scd_ranks << "scd" << m_ranks_matrixes.scd_matrix;
//	save_scd_ranks.release();
//
//	cv::FileStorage save_ehd_ranks("ehd_ranks_f.xml", cv::FileStorage::WRITE);
//	save_ehd_ranks << "ehd" << m_ranks_matrixes.ehd_matrix;
//	save_ehd_ranks.release();
//
//	cv::FileStorage save_cd_ranks("cd_ranks_f.xml", cv::FileStorage::WRITE);
//	save_cd_ranks << "cd" << m_ranks_matrixes.cd_matrix;
//	save_cd_ranks.release();
}
