#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <vector>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/ximgproc.hpp>

#include <opencv2/highgui/highgui.hpp>

namespace preprocessing
{

	/* prepinace
	scaling: -sc, --scale, parametre: DPI, method;
	denoise: -n, --denoise, parametre:
	greyscale: -g, --greyscale, parametre:
	binarize: -b, --binarize, parametre:
	skew correction: -sk, --deskew, parametre:
	enhance: -e, --enhance,

	*/

	enum scale_method {};

	enum greyscale_method { AVG, LUMA, SINGLE_R, SINGLE_G, SINGLE_B, DESATURATE };

	enum binarization_method { GLOBAL, OTSU, ADAP_MEAN, ADAP_GAUS, NIBLACK, SAUVOLA, BERNSEN };

	enum denoise_method { GAUSSIAN, MEAN, WIENER, MEDIAN, NON_LOCAL };

	class config
	{
	public:
		config();

		// scale parameters
		int sc_dpi;
		scale_method sc_method;

		std::vector<denoise_method> noise_methods;

		// deskew parameters

		greyscale_method gs_method;

		binarization_method bin_method;

		// processing argument vector

		std::vector<cv::Mat> files;// processing file vector`

	};

	std::string get_filename(std::string & input_path);

	bool is_number(const std::string& str);

	bool process_scale_arg(const std::string & arg, config & cfg);

	bool process_binar_arg(std::string & arg, config & cfg);

	bool process_greyscale_arg(std::string & arg, config & cfg);

	bool process_denoise_arg(std::string & arg, config & cfg);

    config parse_args(int argc, char* argv[]);
    
}