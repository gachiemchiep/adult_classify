//============================================================================
// Name        : main.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

template<typename VarType> bool isSkinPixel(VarType Y, VarType Cr, VarType Cb) {
	bool bResult;

//	VarType dShita1, dShita2, dShita3, dShita4;
//	if (Y > 128) {
//		dShita1 = -2 + (256 - Y) / 16;
//		dShita2 = 20 - (256 - Y) / 16;
//		dShita3 = 6;
//		dShita4 = -8;
//	} else {
//		dShita1 = 6;
//		dShita2 = 12;
////		dShita3 = 20 - (256 - Y) / 16;
//		dShita3 = 20 +  Y / 32;
//		dShita4 = -16 + (Y / 16);
//	}
//
//	vector<VarType> viBottomLims = { -2 * (Cb + 24), -(Cb + 17), -4 * (Cb + 32),
//			2.5 * (Cb + dShita1), dShita3, 0.5 * (dShita4 - Cb) };
//	vector<VarType> viTopLims = { (220 - Cb) / 6, 4/3 * (dShita2 - Cb)};
//
//	VarType tBottomLim = *max_element(viBottomLims.begin(), viBottomLims.end());
//	VarType tTopLim = *min_element(viTopLims.begin(), viTopLims.end());
//
//	if ((Cr >= tBottomLim) && (Cr <= tTopLim)) {
//		bResult = true;
//	} else {
//		bResult = false;
//	}

	if ( (Cr >= 80) && (Cb <= 135) && (Cr >=131) && (Cr <= 185) ) {
		bResult = true;
	} else {
		bResult = false;
	}

	return bResult;
}

int main(int argc, char *argv[]) {

	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << "image location \n";
		return -1;
	}

	Mat mFrame = cv::imread(argv[1], 1);

	// simple RGB rule to get skin
	vector<Mat> vmFrame;
	Mat mFrame2 = mFrame.clone();
	for (int h = 0; h < mFrame.rows; h++) {
		for (int w = 0; w < mFrame.cols; w++) {
			int iBlue = mFrame.at<Vec3b>(h, w)[0];
			int iGreen = mFrame.at<Vec3b>(h, w)[1];
			int iRed = mFrame.at<Vec3b>(h, w)[2];
			int iMax = max(iBlue, iGreen);
			iMax = max(iMax, iRed);
			int iMin = min(iBlue, iGreen);
			iMin = min(iMin, iRed);
			if ((iRed > 95) && (iGreen > 40) && (iBlue > 20)
					&& ((iMax - iMin) > 15) && (abs(iRed - iGreen) > 15)
					&& (iRed > iGreen) && (iRed > iBlue)) {
				// is a skin pixel
//				mFrame2.at<Vec3b>(h, w) = mFrame.at<Vec3b>(h, w);
			} else {
				// not a skin pixel
				mFrame2.at<Vec3b>(h, w) = Vec3b(0, 0, 0);
			}
		}
	}

	imshow("skin only", mFrame2);
	Mat mFrameYCrCb;
	cvtColor(mFrame, mFrameYCrCb, CV_BGR2YCrCb);
	Mat mFrameCrop = mFrame.clone();
	for (int h = 0; h < mFrame.rows; h++) {
		for (int w = 0; w < mFrame.cols; w++) {
			int fY = mFrameYCrCb.at<Vec3b>(h, w)[0];
			int fCr = mFrameYCrCb.at<Vec3b>(h, w)[1];
			int fCb = mFrameYCrCb.at<Vec3b>(h, w)[2];
			bool bSkin = isSkinPixel<double>(fY, fCr, fCb);
			if (bSkin) {

			} else {
				mFrameCrop.at<Vec3b>(h, w) = Vec3b(0, 0, 0);
			}
		}
	}

	imshow("Another defined rule", mFrameCrop);
	waitKey(0);

	return 0;
}

