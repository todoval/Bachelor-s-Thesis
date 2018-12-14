#pragma once

#include "parser.h"
#include "opencv2/core.hpp"
#include "opencv2/photo/photo.hpp"

namespace preprocessing
{

	struct RGB
	{
		uchar blue;
		uchar green;
		uchar red;
	};

	using resolution = std::pair<int,int>;

	void determine_cfg(cv::Mat &img, config & cfg);

	cv::Mat preprocess_img(cv::Mat &img, config & cfg);

	resolution get_new_resolution(cv::Mat &img, int old_dpi, int new_dpi);

	cv::Mat& scale(cv::Mat &img, int dpi, scale_method method);

	cv::Mat& grey_conversion(cv::Mat &img, greyscale_method method);

	cv::Mat& binarize(cv::Mat &img, binarization_method method);

	cv::Mat& denoise(cv::Mat &img, denoise_method & method);

	void savuola(cv::Mat & img, int k);

	void deskew(cv::Mat & img, double angle);

	int get_skew(cv::Mat &img);

	bool is_skewed(cv::Mat &img);

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