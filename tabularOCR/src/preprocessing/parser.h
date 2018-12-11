#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <vector>
#include "opencv2/imgproc/imgproc.hpp"

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

	enum greyscale_method { SIMPLE, LUMA, SINGLE_R, SINGLE_G, SINGLE_B, DESATURATE };

	class config
	{
	public:
		config();

		// scale parameters
		int sc_dpi;
		scale_method sc_method;

		// denoise parameters

		// deskew parameters

		// greyscale parameters
		greyscale_method gs_method;

		// binarization parameters

		// processing argument vector

		std::vector<cv::Mat> files;// processing file vector`

	};

	std::string get_filename(std::string & input_path);

	bool is_number(const std::string& str);

	bool process_scale_arg(const std::string & arg, config & cfg);

	/*
	parse the input arguments into a ocr::Process_info
	*/
    config parse_argss(int argc, char* argv[]);
    
}