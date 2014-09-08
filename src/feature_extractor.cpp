/*
 * feature_extractor.cpp
 *
 *  Created on: Aug 27, 2014
 *      Author: gachiemchiep
 */

#include "feature_extractor.h"

feature_extractor::feature_extractor() {
	// TODO Auto-generated constructor stub
}

feature_extractor::~feature_extractor() {
	// TODO Auto-generated destructor stub
}

/**
 * Set image path for feature extraction
 */
void feature_extractor::set_img_path(std::string img_path) {
	m_img = img_path;
}

/**
 * Get image path for feature extraction
 */
std::string feature_extractor::get_img_path() {
	return m_img;
}

/**
 * Set method for feature extraction
 */
void feature_extractor::set_method(std::string method) {
	m_method = method;
}

/**
 * Get method for feature extraction
 */
std::string feature_extractor::get_method() {
	return m_method;
}

/**
 * Get extracted feature
 */
cv::Mat feature_extractor::get_feature() {
	return m_feature;
}

/**
 * Check whether input image valid
 */
bool feature_extractor::check_img_path() {
	cv::Mat tmp = cv::imread(m_img, 1);
	if (tmp.data != NULL) {
		return true;
	} else {
		return false;
	}
}

/**
 * Check whether input method valid
 */
bool feature_extractor::check_method() {
	if (std::find(EXTRACT_FEATURE_METHODS.begin(), EXTRACT_FEATURE_METHODS.end(), m_method) == EXTRACT_FEATURE_METHODS.end()) {
		std::cerr << m_method << " is not valid. \n";
		std::cerr << 'Valid method "SCD", "EHD", "CD", "ALL"\n';
		std::cerr << "Should use ALL only \n";
		return false;
	} else {
		return true;
	}
}

/**
 * Check whether input method and input image valid
 */
bool feature_extractor::is_valid() {
	if ((check_img_path()) && (check_method())) {
		std::cerr << m_img << " " << m_method << " is usable \n";
		return true;
	} else {
		std::cerr << m_img << " Or " << m_method << " is not valid \n";
		return false;
	}
}

/**
 * Calculate feature
 */
void feature_extractor::calculate_feature() {
	if (is_valid()) {
		// read m_img as bgr matrix
		m_frame = cv::imread(m_img, 1);
		// TODO feature calculationg
		cv::Mat scd = calculate_scd(m_frame);
		cv::Mat ehd = calculate_ehd(m_frame);
		cv::Mat cd = calculate_cd(m_frame);

		// 256 + 85 + 21
		int cols = scd.cols + ehd.cols + cd.cols;
		m_feature = cv::Mat::zeros(1, cols, CV_16U);
		// merge into 1 feature to save somewhere
		for (int i = 0; i < m_feature.cols; i++) {
			if (i < scd.cols) { //scd
				m_feature.at<unsigned short>(0, i) = scd.at<unsigned short>(0, i);
			} else if (i < (scd.cols + ehd.cols)) { // ehd
				m_feature.at<unsigned short>(0, i) = ehd.at<unsigned short>(0, i - scd.cols) ;
			} else { // cd
				m_feature.at<unsigned short>(0, i) = cd.at<unsigned short>(0, i - scd.cols - ehd.cols) ;
			}
		}
	} else {
		std::cerr << "Image or method is not valid \n";
	}
}

/**
 * Calculate Scalable color descriptor
 * opencv hsv is [0-179],[0-255],[0-255] for 8bit image
 * Algorithm
 * scd = hsv - 16x4x4 = 256 dimension matrix
 * Count and assigne hsv value into corresponding bin
 * s[1], s[2], ...
 * s[1] =  hsv [1, 1, 1]
 * s[4] = hsv [1, 1, 4]
 * s[5] = hsv [1, 2, 1]
 * s[256] = hsv [16, 4, 4]
 *
 */
cv::Mat feature_extractor::calculate_scd(cv::Mat &frame) {

	cv::Mat frame_hsv;
	cv::cvtColor(frame, frame_hsv, CV_BGR2HSV);
	cv::Mat scd = cv::Mat::zeros(1, 256, CV_32F);
	for (int h = 0; h < frame_hsv.rows; h++) {
		for (int w = 0; w < frame_hsv.cols; w++) {
			cv::Vec3b hsv = frame_hsv.at<cv::Vec3b>(h, w);
			int h_bin = hsv[0] / 15;
			int s_bin = hsv[1] / 64;
			int v_bin = hsv[2] / 64;
			int position = 16 * h_bin + 4 * s_bin + v_bin;
			scd.at<float>(0, position) += 1;
		}
	}
	// TODO normalize feature then quantitize
//	std::cerr << frame.size().area() << "\n";
//	std::cerr << cv::sum(scd) << "\n";
//	std::cerr << scd;
	scd = scd / frame.size().area() * NORM_SIZE;
	scd.convertTo(scd, CV_16U, 1, 0);
	return scd;
}

/**
 * Calculate ehd features descriptor
 * 16x5(local) + 1x5(global) = 85 dimension
 * Algorithm
 * image -> 4x4 sub_image
 * sub_image -> 1024 image_block
 * 	1024 = 32^2 , image_block_w = image_width / 32
 * 				  image_block_h = image_height / 32
 * image_block -> 2x2 sub_image_block (0, 1, 2, 3)
 * for each sub_image_block -> gray value -?
 * use equation 3-7 to calculate edge streng
 * compare with threshold to find edge type
 *
 * Read Dong's "Efficient use of local edge histogram" for more detail
 * To make it easier to calculate
 * resize image so that width and height chia het cho 8
 *
 */
cv::Mat feature_extractor::calculate_ehd(cv::Mat &frame) {
	cv::Mat ehd = cv::Mat::zeros(1, 85, CV_32F);
	int rate = std::sqrt(EHD_BLOCK_COUNT);

	// image -> 4x4 sub_image
	// resize
	int img_h = frame.rows;
	int img_w = frame.cols;
	int new_img_h = (img_h / (rate * 2) + 1) * (rate *2 );
	int new_img_w = (img_w / (rate * 2) + 1) * (rate * 2);
	cv::Mat new_frame;
	cv::resize(frame, new_frame, cv::Size(new_img_w, new_img_h));

	cv::Mat new_frame_gray;
	cv::cvtColor(new_frame, new_frame_gray, CV_BGR2GRAY);

	std::vector<cv::Rect> sub_frames;
	cv::Rect entire_img = cv::Rect(0, 0, new_img_w, new_img_h);

	// local ehd
	sub_frames = split_rect(entire_img, 4, 4);
	for (unsigned i = 0; i < sub_frames.size(); i++) {
		cv::Rect sub_frame = sub_frames[i];
		std::vector<cv::Rect> sub_frame_blocks;
		sub_frame_blocks = split_rect(sub_frame, rate/4, rate/4);
		for (unsigned j = 0; j < sub_frame_blocks.size(); j++) {
			std::vector<float> edges = edges_streng(new_frame_gray, sub_frame_blocks[j]);
			float strong_edge = strongest_edge(edges, EDGE_THRES);
			if (strong_edge == 0) {
				// do not have edge
			} else {
				std::vector<float>::iterator index =
				std::find(edges.begin(), edges.end(), strong_edge);
				int edge_type = index - edges.begin() ;
				int descriptor_position = 5 * i + edge_type;
				ehd.at<float>(0, descriptor_position) += 1;
			}

		}
	}
	//normalize : local ehd is extracted from rate * rate block
	for (int i = 0; i < 80; i++) {
		ehd.at<float>(0, i) = ehd.at<float>(0, i) / (rate * rate);
	}

	// global ehd
	std::vector<cv::Rect> frame_blocks;
	frame_blocks = split_rect(entire_img, 2, 2);
	for (unsigned j = 0; j < frame_blocks.size(); j++) {
		std::vector<float> edges = edges_streng(new_frame_gray, frame_blocks[j]);
		float strong_edge = strongest_edge(edges, EDGE_THRES);
		if (strong_edge == 0) {
			// do not have edge
		} else {
			std::vector<float>::iterator index =
			std::find(edges.begin(), edges.end(), strong_edge);
			int edge_type = index - edges.begin() ;
			int descriptor_position = 5 * 16 + edge_type;
			ehd.at<float>(0, descriptor_position) += 1;
		}
	}
	//normalize : global ehd is extracted from 2x2
	for (int i = 80; i < 85; i++) {
		ehd.at<float>(0, i) = ehd.at<float>(0, i) / (2 * 2);
	}

	// TODO normalize then quantitize
	// block's number = rate * rate
	ehd = ehd * NORM_SIZE;
	ehd.convertTo(ehd, CV_16U, 1, 0);
	return ehd;
}

/**
 * Calculate edge streng for ehd feature descriptors
 * edge type is ver, hor, dia45, dia135, nond_edge
 * For detail read Dong's Efficient use of MPEG-7 Edge
 * Histogram descriptor
 */
std::vector<float> feature_extractor::edges_streng(cv::Mat &frame_gray,
		cv::Rect block_rect) {
	std::vector<float> edges_streng { 0, 0, 0, 0, 0 };
	// block_rect -> 0, 1, 2, 3 smaller blocks
	if (((block_rect.width % 2) != 0) || ((block_rect.height % 2) != 0)) {
		std::cerr << "Input block size is not valid \n";
		std::cerr << "block's height and width should be divisible by 2 \n";
		return edges_streng;
	} else {
		std::vector<cv::Rect> sub_rect;
		sub_rect = split_rect(block_rect, 2, 2);
		// sub block average gray level
		for (unsigned count = 0; count < EDGE_FILTERS.size(); count ++) {
			std::vector<float> edge_filter = EDGE_FILTERS[count];
			// calculate average gray in 0,1,2,3 image_block
			std::vector<float> subs_gray;
			for (unsigned i = 0; i < sub_rect.size(); i++) {
				cv::Scalar gray_mean = cv::mean(frame_gray(sub_rect[i]));
				subs_gray.push_back(gray_mean[0]);
			}
			float edge_streng = multiply_scalars(subs_gray, edge_filter);
			edges_streng[count] = edge_streng;
		}

	}
	return edges_streng;
}

/**
 * After calculating edges's strength
 * Find the strongest edge velue
 */
float feature_extractor::strongest_edge(std::vector<float>& edges_streng, float threshold) {
	float highest_edge = maximum(edges_streng);
	if (highest_edge > threshold) {
		return highest_edge;
	} else {
		return 0;
	}
}

/**
 * Divide input rect into smaller part
 * for x axis : divide into x_part_counts
 * for y axis : divide into y_part_counts
 * rect's height, width should be divisible by y_part_counts, x_part_counts
 * return value should be absolute value coresponding to frame's coordinates
 */
std::vector<cv::Rect> feature_extractor::split_rect(cv::Rect rect,
		int y_parts_count, int x_parts_count) {
	std::vector<cv::Rect> smaller_rects;

	if (((rect.width % x_parts_count) != 0)
			|| ((rect.height % y_parts_count) != 0)) {
		std::cerr << "Input block size is not valid \n";
		std::cerr
				<< "block's height and width should be divisible by y_parts_count, and x_part_counts \n";
		std::cerr << "Rect's height " << rect.height << " Rect's width "
				<< rect.width << "\n";
		std::cerr << "y_parts_count: " << y_parts_count << " x_parts_count: "
				<< x_parts_count << "\n";
	} else {
		int smaller_rect_w = rect.width / x_parts_count;
		int smaller_rect_h = rect.height / y_parts_count;
		for (int y = 0; y < y_parts_count; y++) {
			for (int x = 0; x < x_parts_count; x++) {
				int smaller_rect_x = rect.x + x * smaller_rect_w;
				int smaller_rect_y = rect.y + y * smaller_rect_h;
				cv::Rect smaller_rect = cv::Rect(smaller_rect_x, smaller_rect_y,
						smaller_rect_w, smaller_rect_h);
				smaller_rects.push_back(smaller_rect);
			}
		}
	}

	return smaller_rects;
}
/**
 * Calculate compactness descriptor(CD)
 * algorithm
 * divide image into 1 , 4 , 16 sub_image
 * then count the number of skin pixel in each subimage
 * rate = skin_pixels / area
 * the push into descriptor
 *  1           4             16
 * cd[1], cd[2],..,cd[5],cd[5],..cd[21]
 * switching method to calculate this make me dizzy
 * TODO: add later
*/

cv::Mat feature_extractor::calculate_cd(cv::Mat &frame) {
	cv::Mat cd_descriptor = cv::Mat::zeros(1, 21, CV_32F);
	// convert frame to divisible 4x4
	// image -> 4x4 sub_image
		// resize
	int img_h = frame.rows;
	int img_w = frame.cols;
	int new_img_h = (img_h / 4 + 1) * 4;
	int new_img_w = (img_w / 4 + 1) * 4;
	cv::Mat new_frame;
	cv::resize(frame, new_frame, cv::Size(new_img_w, new_img_h));

	cv::Mat skin_map = cv::Mat::zeros(new_frame.rows, new_frame.cols, CV_32F);
	for (int h = 0; h < new_img_h; h++) {
		for (int w = 0; w < new_img_w; w++) {
			cv::Vec3b pixel = new_frame.at<cv::Vec3b>(h, w);
			if (is_skin_pixel(pixel)) {
				skin_map.at<float>(h, w) = 1;
			}
		}
	}

	cv::Rect entire_img = cv::Rect(0, 0, new_img_w, new_img_h);
	cd_descriptor.at<float>(0, 0) = cv::sum(skin_map)[0] / skin_map.size().area();
	std::vector<cv::Rect> sub_img1 = split_rect(entire_img, 2, 2);
	std::vector<float> compact1 ;
	std::vector<cv::Rect> sub_img2 = split_rect(entire_img, 4, 4);
	std::vector<float> compact2 ;
	for (unsigned j = 0; j < sub_img1.size(); j++) {
		compact1.push_back(cv::sum(skin_map(sub_img1[j]))[0] / skin_map.size().area());
	}
	// descending order
	std::sort(compact1.rbegin(), compact1.rend());
	for (unsigned j = 0; j < sub_img2.size(); j++) {
		compact2.push_back(cv::sum(skin_map(sub_img2[j]))[0] / skin_map.size().area());
	}
	std::sort(compact2.rbegin(), compact2.rend());
	for (int i = 1; i < 5; i++) {
		cd_descriptor.at<float>(0, i) = compact1[i-1];
	}
	for (int i = 5; i < 21; i++) {
		cd_descriptor.at<float>(0, i) = compact2[i-5];
	}
	// TODO: norm
	cd_descriptor = cd_descriptor * NORM_SIZE;
	cd_descriptor.convertTo(cd_descriptor, CV_16U, 1, 0);
	return cd_descriptor;
}



/**
 * Is input pixel a skin pixel
 * Using simple RGB rule
 */
bool feature_extractor::is_skin_pixel(cv::Vec3b bgr) {
	int blue = bgr[0];
	int green = bgr[1];
	int red = bgr[2];
	int max = std::max(blue, green);
	max = std::max(max, red);
	int min = std::min(blue, green);
	min = std::min(min, red);
	if ((red > 95) && (green > 40) && (blue > 20) && ((max - min) >= 15)
			&& (std::abs(red - green) >= 15) && (red > blue) && (red > green)) {
		return true;
	} else if ((red > 220) && (green > 210) && (blue > 170)
			&& (std::abs(red - green) <= 15) && (blue < red)
			&& (blue < green)) {
		return true;
	} else {
		return false;
	}
}

/**
 * Save extracted feature into result_file as raw txt file
 * Delimiter is ","
 */
void feature_extractor::save_result(std::string result_file) {
	save_feature(m_feature, m_img, result_file);
}

/**
 * Save image_path, extracted feature into result_file as raw txt file
 * For example : image1.jpg,1,2,4,233,...
 */
void feature_extractor::save_feature(cv::Mat &feature, std::string file_name, std::string result_file) {
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
