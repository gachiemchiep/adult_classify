/*
 * main.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: gachiemchiep
 *      Description : main
 */

#include <iostream>
#include <stdio.h>
#include "feature_extractor.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {

	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << "image location \n";
		return -1;
	}

	std::string img_loc = std::string(argv[1]);
	feature_extractor fe ;
	fe.set_img_path(img_loc);
	fe.set_method("ALL");
	fe.calculate_feature();

	cv::waitKey(0);
	return 0;
}

