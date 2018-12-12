#pragma once

#include "parser.h"

namespace preprocessing
{

	struct RGB
	{
		uchar blue;
		uchar green;
		uchar red;
	};

	using resolution = std::pair<int,int>;

	cv::Mat preprocess_img(cv::Mat &img);

	resolution get_new_resolution(cv::Mat &img, int old_dpi, int new_dpi);

	cv::Mat& scale(cv::Mat &img, int dpi, scale_method method);

	cv::Mat& grey_conversion(cv::Mat &img, greyscale_method method);

	cv::Mat& binarize(cv::Mat &img, binarization_method method);

	cv::Mat& denoise(cv::Mat &img, std::vector<denoise_method> & methods);

	/*
	to do:
	denoise
	skew correction
	scaling
	constrast enhancement
	binarization
	marginal noise
	- other
	*/


}