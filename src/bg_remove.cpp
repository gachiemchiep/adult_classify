/*
 * BackgroundRemoval.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: gachiemchiep
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
}

void bg_remove::show_crop(int i) {
	cv::imshow(v_methods[i], m_crop_mat);
}

void bg_remove::save_crop(int i) {
	std::string method = v_methods[i];
	boost::filesystem::path img_pth(m_img_path);
	std::string crop_name = img_pth.stem().string() + "_" + method
			+ img_pth.extension().string();
	boost::replace_all(m_img_path, img_pth.filename().string(), crop_name);
	std::cerr << m_img_path << "\n";
	// TODO: save somewhere,
	// make a command list to run
//	cv::imwrite(m_crop_mat, "where?");
}

void bg_remove::show_bin(int i) {
	remove_noise();

	cv::imshow(v_methods[i], m_crop_bin);
}

void bg_remove::remove_noise() {
	cv::cvtColor(m_crop_mat, m_crop_bin, CV_BGR2GRAY);
	cv::threshold(m_crop_bin, m_crop_bin, 100, 255, cv::THRESH_BINARY);
	// divide image into 16x16 non-overlapping block
	// if block has more than 50% skin , then that block is skin block
	find_skin_block();
	//closing mornopoly to connect skin blocks
	connect_skin_block();
	// find largest ractangle and remoev the others
	find_largest_skin_block();
	// finally crop mat again to find ROI
}

void bg_remove::find_skin_block() {
	int width = m_crop_bin.cols;
	int height = m_crop_bin.rows;
	int new_width = (width / BLOCK_SIZE.width + 1) * BLOCK_SIZE.width;
	int new_height = (height / BLOCK_SIZE.height + 1) * BLOCK_SIZE.height;

	printf("old: %d, %d, new: %d, %d\n", height, width, new_height, new_width);
	cv::resize(m_crop_bin, m_crop_bin, cv::Size(new_width, new_height));

	cv::Mat m_block_empty = cv::Mat::zeros(BLOCK_SIZE.width, BLOCK_SIZE.height, CV_8UC1);

	std::vector<cv::Rect> skin_blocks;
	for (int y = 0; y < (new_height / BLOCK_SIZE.height); y++) {
		for (int x = 0; x < (new_width / BLOCK_SIZE.width); x++) {
			cv::Rect tmp_rect(x * BLOCK_SIZE.width, y * BLOCK_SIZE.height,
					BLOCK_SIZE.width, BLOCK_SIZE.height);
			if (is_skin_block(tmp_rect, m_crop_bin)) {
				skin_blocks.push_back(tmp_rect);
				cv::rectangle(m_img_mat, tmp_rect.tl(), tmp_rect.br(),
						cv::Scalar(255, 0, 255), 1, 8, 0);
			} else {
				m_block_empty.copyTo(m_crop_bin(tmp_rect));
			}
		}
	}
	imshow("new_size", m_img_mat);
	imshow("remove non-skin blocks", m_crop_bin);
}

bool bg_remove::is_skin_block(cv::Rect block_r, cv::Mat &m_crop_bin_fixed) {
	int count = 0;
	for (int h = block_r.y; h < (block_r.y + block_r.height); h++) {
		for (int w = block_r.x; w < (block_r.x + block_r.width); w++) {
			if (m_crop_bin_fixed.at<uchar>(h, w) == 255) {
				count++;
			}
		}
	}
	int count_min = (BLOCK_SIZE.width * BLOCK_SIZE.height) / 2;
	if (count > count_min) {
		return true;
	} else {
		return false;
	}
}

void bg_remove::connect_skin_block() {

}

void bg_remove::find_largest_skin_block() {

}

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

bool bg_remove::is_valid_method() {
	if (std::find(v_methods.begin(), v_methods.end(), m_method)
			== v_methods.end()) {
		std::cerr << m_method << " is not valid. \n";
		std::cerr
				<< 'Valid method "RGB", "YCrCb", "HSV1", "HSV2", "RGB_norm", "HSI"';
		return false;
	} else {
		return true;
	}
}

void bg_remove::init() {
	if (is_valid_img()) {
		std::cerr << "Running \n";

	} else {
		std::cerr << "Try again \n";
		return;
	}
}

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

bool ycrcb::is_skin_pixel(cv::Vec3b ycrcb) {
	int cr = ycrcb[1];
	int cb = ycrcb[2];
	if ((cb >= 77) && (cb <= 127) && (cr >= 133) && (cr <= 173)) {
		return true;
	} else {
		return false;
	}
}

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

// h:0->180, s:0->255, v:0->255
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
