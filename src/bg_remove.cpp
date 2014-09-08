/**
 * @file BackgroundRemoval.cpp
 *
 *  Created on: Aug 21, 2014
 *  Author: gachiemchiep
 */

#include "bg_remove.h"

bg_remove::bg_remove() {
	// TODO Auto-generated constructor stub

}

bg_remove::~bg_remove() {
	// TODO Auto-generated destructor stub
}

void bg_remove::set_img(std::string &img) {
	m_img_path = img;
	std::cerr << "Input image:" << m_img_path << "\n";
}

void bg_remove::show_crop(int i) {
	cv::imshow(RM_BG_METHODS[i] + "_crop", m_crop_mat);
}

void bg_remove::run() {
	remove_noise();
	crop_img(m_roi);
}

/**
 * Save background removed image base on method
 */
void bg_remove::save_crop(int i) {

	// save somewhere
	std::string method = RM_BG_METHODS[i];
	boost::filesystem::path img_pth(m_img_path);
	std::string crop_name = img_pth.stem().string() + "_" + method
			+ img_pth.extension().string();
	boost::replace_all(m_img_path, img_pth.filename().string(), crop_name);
//		std::cerr << m_img_path << "\n";
	cv::imwrite(m_img_path, m_crop_mat);
}

/**
 * Whether ROI(region of interest) sastified image's size condition
 * Size :(height > 50) && (width > 50)
 */
cv::Rect bg_remove::analyze_roi(cv::Rect largest_block) {
	int area = largest_block.area();
	if ((area < (m_img_mat.size().area() * MIN_OVERLAP))
			|| (largest_block.width < 50) || (largest_block.height < 50)) {
		cv::Rect new_roi = cv::Rect(0, 0, m_img_mat.cols, m_img_mat.rows);
		return new_roi;
	} else {
		return largest_block;
	}
}

/**
 * Remove non-skin pixels from input image
 */
void bg_remove::crop_img(cv::Rect &roi) {
	m_crop_mat = m_img_mat(roi);
	// crop binary image
	for (int h = 0; h < m_crop_bin.size().height; h++) {
		for (int w = 0; w < m_crop_bin.size().width; w++) {
			if (((h < roi.y)
				|| (h > (roi.y + roi.height)))
					&& ((w < roi.x)
						|| (w > (roi.x + roi.width)))) {
				m_crop_bin.at<uchar>(h, w) = 0;
			}
		}
	}
}

/**
 * Show non-skin pixels removed binary image
 */
void bg_remove::show_bin(int i) {
	cv::imshow(RM_BG_METHODS[i] + "_bin", m_crop_bin);
}

void bg_remove::remove_noise() {
	cv::cvtColor(m_crop_mat, m_crop_bin, CV_BGR2GRAY);
	cv::threshold(m_crop_bin, m_crop_bin, 0, 255, cv::THRESH_BINARY);
	// divide image into 16x16 non-overlapping block
	// if block has more than 50% skin , then that block is skin block
	find_skin_block();
	//closing mornopoly to connect skin blocks
	connect_skin_block();
	// find largest ractangle and remove the others
	find_largest_skin_block();
	m_roi = analyze_roi(m_roi);
}

/**
 * Find skin pixel blocks
 */
void bg_remove::find_skin_block() {
	int width = m_crop_bin.cols;
	int height = m_crop_bin.rows;
	int new_width = (width / BLOCK_SIZE.width + 1) * BLOCK_SIZE.width;
	int new_height = (height / BLOCK_SIZE.height + 1) * BLOCK_SIZE.height;

	cv::resize(m_crop_bin, m_crop_bin, cv::Size(new_width, new_height));
	cv::resize(m_img_mat, m_img_mat, cv::Size(new_width, new_height));

	cv::Mat m_block_empty = cv::Mat::zeros(BLOCK_SIZE.width, BLOCK_SIZE.height,
	CV_8UC1);

	std::vector<cv::Rect> skin_blocks;
	for (int y = 0; y < (new_height / BLOCK_SIZE.height); y++) {
		for (int x = 0; x < (new_width / BLOCK_SIZE.width); x++) {
			cv::Rect tmp_rect(x * BLOCK_SIZE.width, y * BLOCK_SIZE.height,
					BLOCK_SIZE.width, BLOCK_SIZE.height);
			if (!is_skin_block(tmp_rect, m_crop_bin)) {
				m_block_empty.copyTo(m_crop_bin(tmp_rect));
			}
		}
	}
}

/**
 * Whether skin pixel blocks really a skin block
 * Condition: have more than 25% region of block
 *            is skin pixels
 */
bool bg_remove::is_skin_block(cv::Rect block_r, cv::Mat &m_crop_bin_fixed) {
	int count = 0;
	for (int h = block_r.y; h < (block_r.y + block_r.height); h++) {
		for (int w = block_r.x; w < (block_r.x + block_r.width); w++) {
			if (m_crop_bin_fixed.at<uchar>(h, w) == 255) {
				count++;
			}
		}
	}
	int count_min = (BLOCK_SIZE.width * BLOCK_SIZE.height) / 4;
	if (count > count_min) {
		return true;
	} else {
		return false;
	}
}

/**
 * After removing non-pixel blocks, connect blocks using
 * close transformation
 */
void bg_remove::connect_skin_block() {
	cv::morphologyEx(m_crop_bin, m_crop_bin, cv::MORPH_CLOSE,
			MORPH_CLOSE_PARAMS);
	cv::threshold(m_crop_bin, m_crop_bin, 0, 255, cv::THRESH_BINARY);
}

/**
 * Find largest block formed after close transformation
 * The other blocks are consider as noise and remove from
 * image
 */
void bg_remove::find_largest_skin_block() {
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierachy;
	cv::Mat m_crop_bin_ct;
	m_crop_bin.copyTo(m_crop_bin_ct);
	cv::findContours(m_crop_bin_ct, contours, hierachy, CV_RETR_TREE,
			CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	// find largest contour, the other is considered as background

	std::vector<cv::Point> largest_contour_poly, contour_poly;
	cv::Rect  tmp_rect;
	std::vector<cv::Point> largest_contour;
	double largest_area, tmp_area;
	if (contours.size() != 0) {
		largest_contour = contours.at(0);
		cv::approxPolyDP(cv::Mat(largest_contour), largest_contour_poly, 3,
				true);
		m_roi = cv::boundingRect(cv::Mat(largest_contour_poly));
		largest_area = m_roi.area();
		for (int i = 1; i < contours.size(); i++) {
			cv::approxPolyDP(cv::Mat(contours.at(i)), contour_poly, 3, true);
			tmp_rect = cv::boundingRect(cv::Mat(contour_poly));
			tmp_area = tmp_rect.area();
			if (tmp_area > largest_area) {
				m_roi = tmp_rect;
				largest_area = tmp_area;
			}
		}
	} else {
		// contour is not found
		std::cerr << "ROI is not found \n" ;
	}

}

/**
 * Is input image valid?
 */
bool bg_remove::is_valid_img() {
	m_img_mat = cv::imread(m_img_path, 1);
	if (m_img_mat.data != NULL) {
		m_crop_mat = m_img_mat.clone();
		return true;
	} else {
		std::cerr << m_img_path << " is not valid \n";
		return false;
	}
}

/**
 * Is input method valid?
 */
bool bg_remove::is_valid_method() {
	if (std::find(RM_BG_METHODS.begin(), RM_BG_METHODS.end(), m_method)
			== RM_BG_METHODS.end()) {
		std::cerr << m_method << " is not valid. \n";
		std::cerr
				<< 'Valid method "RGB", "YCrCb", "HSV1", "HSV2", "RGB_norm", "HSI"';
		return false;
	} else {
		return true;
	}
}

/**
 * Init
 */
void bg_remove::init() {
	if (is_valid_img()) {
		std::cerr << "Running \n";

	} else {
		std::cerr << "Try again \n";
		return;
	}
}

/**
 * non-skin decider base on RGB rule
 */
void rgb::init() {
	if (is_valid_img()) {
		for (int h = 0; h < m_crop_mat.rows; h++) {
			for (int w = 0; w < m_crop_mat.cols; w++) {
				cv::Vec3b pixel = m_crop_mat.at<cv::Vec3b>(h, w);
				if (!is_skin_pixel(pixel)) {
					m_crop_mat.at<cv::Vec3b>(h, w) = cv::Vec3b(0, 0, 0);
				}
			}
		}
	}
}

/**
 * Is a skin pixel base on RGB rule
 */
bool rgb::is_skin_pixel(cv::Vec3b bgr) {
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
 * non-skin decider base on YCrCb rule
 */
void ycrcb::init() {
	if (is_valid_img()) {
		cv::cvtColor(m_crop_mat, m_crop_mat, CV_BGR2YCrCb);
		for (int h = 0; h < m_crop_mat.rows; h++) {
			for (int w = 0; w < m_crop_mat.cols; w++) {
				cv::Vec3b pixel = m_crop_mat.at<cv::Vec3b>(h, w);
				if (!is_skin_pixel(pixel)) {
					m_crop_mat.at<cv::Vec3b>(h, w) = cv::Vec3b(0, 0, 0);
				}
			}
		}
		cv::cvtColor(m_crop_mat, m_crop_mat, CV_YCrCb2BGR);
	}
}

/**
 * Is a skin pixel base on YCrCb rule
 */
bool ycrcb::is_skin_pixel(cv::Vec3b ycrcb) {
	int cr = ycrcb[1];
	int cb = ycrcb[2];
	if ((cb >= 77) && (cb <= 127) && (cr >= 133) && (cr <= 173)) {
		return true;
	} else {
		return false;
	}
}

/**
 * non-skin decider base on RGB normalize rule
 */
void rgb_norm::init() {
	if (is_valid_img()) {
		for (int h = 0; h < m_crop_mat.rows; h++) {
			for (int w = 0; w < m_crop_mat.cols; w++) {
				cv::Vec3b pixel = m_crop_mat.at<cv::Vec3b>(h, w);
				if (!is_skin_pixel(pixel)) {
					m_crop_mat.at<cv::Vec3b>(h, w) = cv::Vec3b(0, 0, 0);
				}
			}
		}
	}
}

/**
 * Is a skin pixel base on RGB normalize rule
 */
bool rgb_norm::is_skin_pixel(cv::Vec3b bgr) {
	float b = float(bgr[0]);
	float g = float(bgr[1]);
	float r = float(bgr[2]);

	float r_norm = r / (b + g + r);
	float b_norm = b / (b + g + r);
	float g_norm = g / (b + g + r);
	float sum = std::pow(r_norm + b_norm + g_norm, 2);
	float param1 = r_norm / g_norm;
	float param2 = (r_norm * b_norm) / sum;
	float param3 = (r_norm * g_norm) / sum;
	if ((param1 > 1.185) && (param2 > 0.107) && (param3 > 0.112)) {
		return true;
	} else {
		return false;
	}
}

/**
 * non-skin decider base on HSV rule
 */
void hsv::init() {
	if (is_valid_img()) {
		cv::cvtColor(m_crop_mat, m_crop_mat, CV_BGR2HSV);
		for (int h = 0; h < m_crop_mat.rows; h++) {
			for (int w = 0; w < m_crop_mat.cols; w++) {
				cv::Vec3b pixel = m_crop_mat.at<cv::Vec3b>(h, w);
				if (!is_skin_pixel(pixel)) {
					m_crop_mat.at<cv::Vec3b>(h, w) = cv::Vec3b(0, 0, 0);
				}
			}
		}
		cv::cvtColor(m_crop_mat, m_crop_mat, CV_HSV2BGR);
	}
}

/**
 * Is a skin pixel base on RGB normalize rule
 * OpenCV range h:0->180, s:0->255, v:0->255
 */
bool hsv::is_skin_pixel(cv::Vec3b hsv1) {
	int h = hsv1[0];
	int s = hsv1[1];
	int v = hsv1[2];

	if ((v >= 40) && (s >= 51) && (s <= 153)) {
		if (((h < 13) && (h > 0)) || ((h > 167) && (h < 180))) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

/**
 * non-skin decider base on HSV rule
 */
void hls::init() {
	if (is_valid_img()) {
		cv::cvtColor(m_crop_mat, m_crop_mat, CV_BGR2HLS);
		for (int h = 0; h < m_crop_mat.rows; h++) {
			for (int w = 0; w < m_crop_mat.cols; w++) {
				cv::Vec3b pixel = m_crop_mat.at<cv::Vec3b>(h, w);
				if (!is_skin_pixel(pixel)) {
					m_crop_mat.at<cv::Vec3b>(h, w) = cv::Vec3b(0, 0, 0);
				}
			}
		}
		cv::cvtColor(m_crop_mat, m_crop_mat, CV_HLS2BGR);
	}
}

/**
 * Is a skin pixel base on HLS  rule
 */
bool hls::is_skin_pixel(cv::Vec3b hls) {
	int h = hls[0];
	int l = hls[1];
	int s = hls[2];

	if ((h < 50) && (s > 25) && (s < 231)) {
		return true;
	} else {
		return false;
	}
}

/**
 * non-skin decider base on HSI rule
 */
void hsi::init() {
	if (is_valid_img()) {
		for (int h = 0; h < m_crop_mat.rows; h++) {
			for (int w = 0; w < m_crop_mat.cols; w++) {
				cv::Vec3b pixel = m_crop_mat.at<cv::Vec3b>(h, w);
				if (!is_skin_pixel(pixel)) {
					m_crop_mat.at<cv::Vec3b>(h, w) = cv::Vec3b(0, 0, 0);
				}
			}
		}
	}
}

/**
 * Is a skin pixel base on HSI  rule
 */
bool hsi::is_skin_pixel(cv::Vec3b bgr) {
	float b = float(bgr[0]);
	float g = float(bgr[1]);
	float r = float(bgr[2]);

	float r_norm = r / (b + g + r);
	float b_norm = b / (b + g + r);
	float g_norm = g / (b + g + r);

	float min = std::min(r_norm, g_norm);
	min = std::min(min, b_norm);
	float s = 1 - 3 * min;
	float i = (r + g + b) / (3 * 255);
	float h = 0.0;
	h = std::acos(
			(0.5 * (2 * r_norm - g_norm - b_norm))
					/ std::sqrt(
							std::pow(r_norm - g_norm, 2)
									+ (r_norm - b_norm) * (g_norm - b_norm)));
	if (b_norm > g_norm) {
		h = 2 * M_PI - h; //h Pi-2Pi
	} else {
		// h: 0-Pi
	}

	// h:0~360, s:0~100, i:0~255
	h = h * 180 / M_PI;
	s = s * 100;
	i = i * 255;

	if ((i > 40) && (s > 13) && (s < 110) && (h > 0) && (h < 28)) {
		return true;
	} else if ((i > 40) && (s > 13) && (s < 75)
			&& (((h > 309) && (h < 331)) || ((h > 332) && (h < 360)))) {
		return true;
	}
//	else if ((h > 332) && (h < 360)) {
//		return true;
//	}
	else {
		return false;
	}
}
