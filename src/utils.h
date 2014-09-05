/*
 * utils.h
 *
 *  Created on: Sep 5, 2014
 *      Author: gachiemchiep
 */

#ifndef UTILS_H_
#define UTILS_H_

template<typename num_type>
void display_vector(std::vector<num_type>& vector_contains) {
	for (int i = 0; i < vector_contains.size(); i++) {
		std::cerr << vector_contains[i] << " ";
	}
	std::cerr << "\n";
}

template<typename numeric_type>
struct nums_pair {
	int position;
	numeric_type value; //*< Value can be rank's score(int); distance(float)*/
//	bool operator <(const nums_pair& np) {
//		return value < np.value;
//	};
//	bool operator >(const nums_pair& np) {
//		return value > np.value;
//	};
};

struct features_distances {
	float scd_distance;
	float ehd_distance;
	float cd_distance;
};

struct matrixes {
	cv::Mat scd_matrix;
	cv::Mat ehd_matrix;
	cv::Mat cd_matrix;
};

template<typename pair>
bool compare_ascending(const pair& a, const pair& b) {
	return a.value < b.value;
}

template<typename pair>
bool compare_descending(const pair& a, const pair& b) {
	return a.value > b.value;
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

// Join two vectors into 1
template<typename num_type>
void join_vector(std::vector<num_type>& vector1,
		std::vector<num_type>& vector2,
		std::vector<num_type>& joined_vector) {
	joined_vector.reserve(
			vector1.size() + vector2.size());
	joined_vector.insert(joined_vector.end(), vector1.begin(),
			vector1.end());
	joined_vector.insert(joined_vector.end(), vector2.begin(),
			vector2.end());
}


#endif /* UTILS_H_ */
