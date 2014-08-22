/*
 * bg_remove.h
 *
 *  Created on: Aug 21, 2014
 *      Author: gachiemchiep
 *      Description : Remove non-skin pixels from image
 *					Using Francesca's skin segmantation using multiple thresholding
 *					Using Khan's "static filtered skin detection"
 *skin pixel rule:
 *			RGB:	R > 95, G > 40, B > 20,
 *					max(B, G, R) - min(B, G, R) > 15
 *					|R - G| > 15, R > G, R > B
 *			YCrCb: Cr >= 77, Cb <= 127, Cr >=133, Cr <= 173
 *			HSV(H=[0,360]): 0<H<25, 335 < H < 360, 0.2 < S <0.6, 40 < V
 *			HSL: 0<h<50, 0.1<s<0.9
 *			HSI: I1 = (R+G+B)/3, I2=(R-B)/2, I3=(2G-R-B)/4
 *				I=I1, S=square_root((I2)^2+(I3)^2),H=tan^-1(I3/I2)
 *				I>40
 *				if 13<S<110 : 0(degree)<H<28, 332<H<360
 *				if 13<S<75: 309<H<331
 *		normalized rgb: r/g>1.185, (r*b)/(r+g+b)^2>0.107,(r*g)/(r+g+b)^2>0.112
 *						r=R/(R+G+B), g=G/(R+G+B), b=B/(R+G+B)
 */

#ifndef BG_REMOVE_H_
#define BG_REMOVE_H_

#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <math.h>
#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>

class bg_remove {
public:
	bg_remove();
	virtual ~bg_remove();
	void set_img(std::string &img_path);
	virtual void init() = 0;
	void show_crop(int i);
	void save_crop(int i);
protected:
	std::string m_img_path;
	std::string m_method = "HSI";
	std::vector<std::string> v_methods {"RGB", "YCrCb", "HSV", "HLS", "RGB_norm", "HSI"};
	cv::Mat m_img_mat;
	cv::Mat m_crop_mat;
	bool is_valid_img();
private:
	bool is_valid_method();
	void save_crop();
};

#endif /* BG_REMOVE_H_ */

class rgb: public bg_remove {
public:
	virtual void init();
private:
	bool is_skin_pixel(cv::Vec3b bgr);
};

class ycrcb: public bg_remove {
public:
	virtual void init();
private:
	bool is_skin_pixel(cv::Vec3b ycrcb);
};

class hsv: public bg_remove {
public:
	virtual void init();
private:
	bool is_skin_pixel(cv::Vec3b hsv1);
};

class hls: public bg_remove {
public:
	virtual void init();
private:
	bool is_skin_pixel(cv::Vec3b hls);
};

class rgb_norm: public bg_remove {
public:
	virtual void init();
private:
	bool is_skin_pixel(cv::Vec3b ycrcb);
};

class hsi: public bg_remove {
public:
	virtual void init();
private:
	bool is_skin_pixel(cv::Vec3b ycrcb);
};

