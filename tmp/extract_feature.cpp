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

using namespace std;
using namespace cv;

void save_feature(cv::Mat &feature, std::string file_name, std::string result_file);

int main(int argc, char *argv[]) {

	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " image_location result_file \n";
		return -1;
	}

	std::string img_loc = std::string(argv[1]);
	std::string result_file = std::string(argv[2]);
	feature_extractor fe ;
	fe.set_img_path(img_loc);
	fe.set_method("ALL");
	cv::Mat feature = fe.calculate_feature();

	save_feature(feature, img_loc,result_file);
	std::cerr << img_loc << "\n";
	// TODO extract feature for all data
	// using RGB is enough

	cv::waitKey(0);
	return 0;
}

void save_feature(cv::Mat &feature, std::string file_name, std::string result_file) {
	if (feature.rows != 1) {
		std::cerr << "Not a valid feature \n";
	} else {
		std::ofstream write_result(result_file.c_str(), std::ios_base::app);
		if (write_result.is_open()) {
			write_result << file_name << ",";
			for (int i = 0; i < feature.cols; i++) {
				if (i == (feature.cols - 1)) {
					write_result << feature.at<unsigned short>(0, i) ;
				} else {
					write_result << feature.at<unsigned short>(0, i) << ",";
				}
			}
			write_result << "\n";
		} else {
			std::cerr << "Unable to open " << result_file << "\n";
		}
		write_result.close();
	}
}
