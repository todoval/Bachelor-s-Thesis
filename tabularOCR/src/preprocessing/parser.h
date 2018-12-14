#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <vector>
#include <map>

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

	enum binarization_method { GLOBAL, OTSU, ADAP_MEAN, ADAP_GAUS, NIBLACK, SAUVOLA, BERNSEN, DEF_B };

	enum denoise_method { GAUSSIAN, MEAN, BILATERAL, MEDIAN, NON_LOCAL };

	class config
	{
	public:
		config();

		// scale parameters
		int sc_dpi;
		scale_method sc_method;

		std::vector<denoise_method> noise_methods;

		std::map<std::string, cv::Mat> files; // files to be processed with their names

		// deskew parameters

		greyscale_method gs_method;

		binarization_method bin_method;

		// processing argument vector

	};

	std::string get_filename(std::string & input_path);

	bool is_number(const std::string& str);

	bool process_scale_arg(const std::string & arg, config & cfg);

	bool process_binar_arg(std::string & arg, config & cfg);

	bool process_greyscale_arg(std::string & arg, config & cfg);

	bool process_denoise_arg(std::string & arg, config & cfg);

    config parse_args(int argc, char* argv[]);
    
}