#include "preprocess.h"

namespace preprocessing
{

	void determine_cfg(cv::Mat &img, config & cfg)
	{
		if (is_skewed(img))
		{
			int skew = get_skew(img);
			deskew(img, skew);

		}
		
		//if (skew == 0)
		{
			if (cfg.bin_method == binarization_method::DEF_B)
				cfg.bin_method = binarization_method::OTSU;

		}
		//else
		{
			if (cfg.bin_method == binarization_method::DEF_B)
			{
				cfg.bin_method = binarization_method::ADAP_GAUS;
			}
			if (cfg.noise_methods.size() == 0)
			{
				cfg.noise_methods.push_back(denoise_method::MEDIAN);
			}
		}
	}

	cv::Mat preprocess_img(cv::Mat &img, config & cfg)
	{
		//scale (img, cfg.sc_dpi, cfg.sc_method);
		grey_conversion(img, cfg.gs_method);
		//cv::equalizeHist(img, img); // enhance contrast
		//determine_cfg(img, cfg);
		//for (size_t i = 0; i < cfg.noise_methods.size(); i++)
			//	denoise(img, cfg.noise_methods[i]);
			//cv::erode(img, img, 255);
			//cv::dilate(img, img, 255);
		cfg.bin_method = binarization_method::OTSU;
		binarize(img, cfg.bin_method);

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
			cv::cvtColor(img, img, cv::COLOR_BGR2GRAY, 1);
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
			cv::cvtColor(img, img, CV_HSV2BGR, 1);
			return img;
		}
		else
		{
			cv::Mat result = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);
			for (size_t x = 0; x < img.size().width; x++)
			{
				for (size_t y = 0; y < img.size().height; y++)
				{
					RGB& rgb = img.ptr<RGB>(y)[x];
					switch (method)
					{
						case greyscale_method::SINGLE_B:
							result.at<uchar>(y, x) = rgb.blue;
							break;
						case greyscale_method::SINGLE_G:
							result.at<uchar>(y, x) = rgb.green;
							break;
						case greyscale_method::SINGLE_R:
							result.at<uchar>(y, x) = rgb.red;
							break;
						case greyscale_method::AVG:
						{
							int new_value = (rgb.blue + rgb.green + rgb.red) / 3;
							result.at<uchar>(y, x) = new_value;
							break;
						}
					}
				}
			}
			img = result;
			return img;
		}
	}

	cv::Mat& binarize(cv::Mat &img, binarization_method method)
	{
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
		{
			cv::Mat dest; // has to be created because of niblack method assertions
			cv::ximgproc::niBlackThreshold(img, dest, 255, cv::ThresholdTypes::THRESH_BINARY_INV, 11, 0);
			img = dest;
			break;
		}
		case (binarization_method::SAUVOLA):
			savuola(img, 3);
			break;
		case (binarization_method::BERNSEN):
			break;
		case(binarization_method::ADAP_MEAN):
			cv::adaptiveThreshold(img, img, 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_MEAN_C, cv::ThresholdTypes::THRESH_BINARY, 5, 0);
			break;
		case(binarization_method::ADAP_GAUS):
			cv::adaptiveThreshold(img, img, 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_GAUSSIAN_C, cv::ThresholdTypes::THRESH_BINARY, 5, 0);
			break;
		}
		return img;
	}

	cv::Mat& denoise(cv::Mat &img, denoise_method & method)
	{
		switch (method)
		{
		case (denoise_method::NON_LOCAL):
			cv::fastNlMeansDenoising(img, img, 3, 7, 21);
			break;
		case (denoise_method::MEDIAN):
			cv::medianBlur(img, img, 7);
			break;
		case (denoise_method::BILATERAL):
			cv::bilateralFilter(img, img, 3, 15, 15);
			break;
		case (denoise_method::MEAN):
			cv::blur(img, img, cv::Size(3, 3));
			break;
		case (denoise_method::GAUSSIAN):
			cv::GaussianBlur(img, img, cv::Size(0, 0), 3);
			break;
		}
		return img;
	}

	void savuola(cv::Mat & img, int k)
	{

		for (int y = 0; y < img.rows; ++y)
		{
			for (int x = 0; x < img.cols; ++x)
			{
				



			}
		}


	}

	void deskew (cv::Mat & img, double angle)
	{
		std::vector<cv::Point> points;
		cv::Mat_<uchar>::iterator it = img.begin<uchar>();
		cv::Mat_<uchar>::iterator end = img.end<uchar>();
		for (; it != end; ++it)
			if (*it)
				points.push_back(it.pos());
		cv::RotatedRect box = cv::minAreaRect(cv::Mat(points));
		cv::Mat rot_mat = cv::getRotationMatrix2D(box.center, angle, 1);
		cv::Mat rotated;
		cv::warpAffine(img, rotated, rot_mat, img.size(), cv::INTER_CUBIC);
		img = rotated;
	}

	int get_skew(cv::Mat &img)
	{
		std::vector<cv::Vec4i> lines;
		cv::Size size = img.size();
		cv::bitwise_not(img, img);
		cv::HoughLinesP(img, lines, 1, CV_PI / 180, 100, size.width / 2, 50);
		cv::Mat disp_lines(size, CV_8UC1, cv::Scalar(0, 0, 0));
		double angle = 0;
		unsigned nb_lines = lines.size();
		for (unsigned i = 0; i < nb_lines; ++i)
		{
			cv::line(disp_lines, cv::Point(lines[i][0], lines[i][1]),
				cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255, 0, 0));
			angle += atan2((double)lines[i][3] - lines[i][1],
				(double)lines[i][2] - lines[i][0]);
		}
		angle /= nb_lines; // mean angle, in radians.
		angle *= (180.0 / CV_PI);
		return angle;
	}

	bool is_skewed(cv::Mat &img)
	{
		// take 10 points from height
		// calculate histogram for each line of its point
		int height = img.size().height;
		int y = 0;
		while (y < height)
		{

			y = y + (height / 100);
		}
		return true;
	}


	// bicubic, bilinear, nearest_neighbor, lanczos, area,


}