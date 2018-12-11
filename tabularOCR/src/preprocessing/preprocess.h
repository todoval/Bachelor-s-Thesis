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

	/**
	 * Returns the resolution of an image if it is to be resized to a different dpi
	 *
	 * @img the image that will be resized
	 * @old_dpi the original dpi of the image that will be changed
	 * @new_dpi the wanted dpi of the new image
	 */
	resolution get_new_resolution(cv::Mat &img, int old_dpi, int new_dpi);

	/*
	

	*/
	cv::Mat scale(cv::Mat &img, int dpi, scale_method method);

	cv::Mat grey_conversion(cv::Mat &img, greyscale_method method);

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