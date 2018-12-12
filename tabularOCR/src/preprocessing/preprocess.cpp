#include "preprocess.h"

namespace preprocessing
{

	cv::Mat preprocess_img(cv::Mat &img)
	{
		return img;
	}

	// TO DO
	resolution get_new_resolution(cv::Mat &img, int old_dpi, int new_dpi)
	{
		int old_width = img.size().width;
		int old_height = img.size().height;
		return resolution(img.size().width, img.size().height);
	}

	// TO DO
	cv::Mat& scale(cv::Mat &img, int dpi, scale_method method)
	{
		// get dpi of the old image
		resolution new_res = get_new_resolution(img, dpi, dpi);
		//cv::resize
		return img;
	}

	cv::Mat& grey_conversion(cv::Mat &img, greyscale_method method)
	{
		if (method == greyscale_method::LUMA)
		{
			cv::cvtColor(img, img, cv::COLOR_RGB2GRAY);
			return img;
		}
		else if (method == greyscale_method::DESATURATE)
		{
			cv::cvtColor(img, img, CV_BGR2HSV);
			for (size_t x = 0; x < img.size().width; x++)
			{
				for (size_t y = 0; y < img.size().height; y++)
					img.ptr<RGB>(y)[x].green = 0;
			}
			cv::cvtColor(img, img, CV_HSV2BGR);
			return img;
		}
		else
		{
			for (size_t x = 0; x < img.size().width; x++)
			{
				for (size_t y = 0; y < img.size().height; y++)
				{
					RGB& rgb = img.ptr<RGB>(y)[x];
					switch (method)
					{
						case greyscale_method::SINGLE_B:
							img.ptr<RGB>(y)[x].green = rgb.blue;
							img.ptr<RGB>(y)[x].red = rgb.blue;
							break;
						case greyscale_method::SINGLE_G:
							img.ptr<RGB>(y)[x].blue = rgb.green;
							img.ptr<RGB>(y)[x].red = rgb.green;
							break;
						case greyscale_method::SINGLE_R:
							img.ptr<RGB>(y)[x].blue = rgb.red;
							img.ptr<RGB>(y)[x].green = rgb.red;
							break;
						case greyscale_method::AVG:
						{
							int new_value = (rgb.blue + rgb.green + rgb.red) / 3;
							img.ptr<RGB>(y)[x].blue = new_value;
							img.ptr<RGB>(y)[x].green = new_value;
							img.ptr<RGB>(y)[x].red = new_value;
							break;
						}
					}
				}			
			}
			return img;
		}
	}

	cv::Mat& binarize(cv::Mat &img, binarization_method method)
	{
		// 

		switch (method)
		{
			case (binarization_method::OTSU):
				// the value 128 is ignored
				cv::threshold(img, img, 128, 255, cv::ThresholdTypes::THRESH_OTSU);
				break;
			case (binarization_method::GLOBAL):
				cv::threshold(img, img, 127, 255, cv::ThresholdTypes::THRESH_BINARY);
				break;
			case (binarization_method::NIBLACK):
				//cv::ximgproc::
				break;
			case (binarization_method::SAUVOLA):
				break;
			case (binarization_method::BERNSEN):
				break;
			case(binarization_method::ADAP_MEAN):
				cv::adaptiveThreshold(img, img, 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_MEAN_C, cv::ThresholdTypes::THRESH_BINARY, 11, 0);
				break;
			case(binarization_method::ADAP_GAUS):
				cv::adaptiveThreshold(img, img, 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_GAUSSIAN_C, cv::ThresholdTypes::THRESH_BINARY, 11, 0);
				break;
		}
		return img;
	}

	cv::Mat& denoise(cv::Mat &img, std::vector<denoise_method> & methods)
	{
		for (size_t i = 0; i < methods.size(); i++)
		{

		}
		return img;
	}

	// bicubic, bilinear, nearest_neighbor, lanczos, area,


}