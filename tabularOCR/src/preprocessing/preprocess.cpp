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
	cv::Mat scale(cv::Mat &img, int dpi, scale_method method)
	{
		// get dpi of the old image
		resolution new_res = get_new_resolution(img, dpi, dpi);
		//cv::resize
		return img;
	}

	cv::Mat grey_conversion(cv::Mat &img, greyscale_method method)
	{
		if (method == greyscale_method::LUMA)
		{
			cv::cvtColor(img, img, cv::COLOR_RGB2GRAY);
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
							continue;
						case greyscale_method::SINGLE_G:
							img.ptr<RGB>(y)[x].blue = rgb.green;
							img.ptr<RGB>(y)[x].red = rgb.green;
							continue;
						case greyscale_method::SINGLE_R:
							img.ptr<RGB>(y)[x].blue = rgb.red;
							img.ptr<RGB>(y)[x].green = rgb.red;
							continue;
						case greyscale_method::SIMPLE:
							img.ptr<RGB>(y)[x].blue = (rgb.blue + rgb.green + rgb.red) / 3;;
							img.ptr<RGB>(y)[x].green = (rgb.blue + rgb.green + rgb.red) / 3;;
							img.ptr<RGB>(y)[x].red = (rgb.blue + rgb.green + rgb.red) / 3;
							continue;
						case greyscale_method::DESATURATE:
							cv::cvtColor(img, img, CV_RGB2HLS);
							RGB& rgb = img.ptr<RGB>(y)[x];
							rgb.green = 0;

							continue;
					}
				}
				
			}
			return img;
		}

	}


	// bicubic, bilinear, nearest_neighbor, lanczos, area,


}