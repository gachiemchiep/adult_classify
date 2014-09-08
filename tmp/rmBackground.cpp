//============================================================================
// Name        : main.cpp
// Author      :
// Version     :
// Copyright   :
// Description : Remove non-skin pixels from image
//				Using Francesca's skin segmantation using multiple thresholding
// skin pixel rule:
//			RGB:	R > 95, G > 40, B > 20,
//					max(B, G, R) - min(B, G, R) > 15
//					|R - G| > 15, R > G, R > B
//			YCrCb: Cr >= 77, Cb <= 127, Cr >=133, Cr <= 173
//			HSV1(H=[0,360]): 0<H<25, 335 < H < 360, 0.2 < S <0.6, 40 < V
//			HSV2(H=[-180,180]): V>=40, H<=(-0.4*V+75),10<=S<=(-H-0.1*V+110)
//								if H >= 0 : S<=(0.08*(100-V)*H+0.5*V)
//								if H<0: S<=(0.5*H+35)
//			HSI: I1 = (R+G+B)/3, I2=(R-B)/2, I3=(2G-R-B)/4
//				I=I1, S=square_root((I2)^2+(I3)^2),H=tan^-1(I3/I2)
//				I>40
//				if 13<S<110 : 0(degree)<H<28, 332<H<360
//				if 13<S<75: 309<H<331
//		normalized rgb: r/g>1.185, (r*b)/(r+g+b)^2>0.107,(r*g)/(r+g+b)^2>0.112
//						r=R/(R+G+B), g=G/(R+G+B), b=B/(R+G+B)
//============================================================================

#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

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

