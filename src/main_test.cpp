/*
 * main.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: gachiemchiep
 *      Description : main
 */

#include <iostream>
#include <stdio.h>
#include "bg_remove.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {

	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << "image location \n";
		return -1;
	}

	std::string img_loc = std::string(argv[1]);

	std::vector<bg_remove*> bg_removes;
	bg_removes.push_back(new rgb());
	bg_removes.push_back(new ycrcb());
	bg_removes.push_back(new hsv());
	bg_removes.push_back(new hls());
	bg_removes.push_back(new rgb_norm());
	bg_removes.push_back(new hsi());

	for (std::vector<bg_remove*>::iterator bg = bg_removes.begin();
			bg != bg_removes.end(); ++bg) {
		(*bg)->set_img(img_loc);
		(*bg)->init();
		int i = bg - bg_removes.begin();
		(*bg)->show_crop(i);
		(*bg)->save_crop(i);
	}

	cv::waitKey(0);
	return 0;
}

