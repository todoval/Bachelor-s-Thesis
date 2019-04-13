#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctype.h>
#include <vector>
#include <map>


#include <baseapi.h>
#include <renderer.h>
#include "allheaders.h"

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

	enum preprocess_method { SCALE, DENOISE, GREYSCALE, BINARIZE, SKEW, ENHANCE, NONE_PRE };

	enum scale_method { NONE_SC };

	enum greyscale_method { AVG, LUMA, SINGLE_R, SINGLE_G, SINGLE_B, DESATURATE, NONE_G };

	enum binarization_method { OTSU, SAUVOLA, NONE_B };

	enum denoise_method { GAUSSIAN, MEAN, BILATERAL, MEDIAN, NON_LOCAL, NONE_N };

	enum enhancement_method { HIST_EQUALIZATION, SIMPLE, GAMMA, NONE_E };

	enum deskew_method {NONE_DSK};

	struct file_info
	{
		std::string name;
		Pix* old;
		Pix* preprocessed;
	};

	class config
	{
	public:
		config();

		// scale parameters
		int sc_dpi;
		scale_method sc_method;
		denoise_method noise_method;
		deskew_method dsk_method;
		greyscale_method gs_method;
		binarization_method bin_method;
		enhancement_method en_method;

		std::vector<file_info> parse_args(int argc, char* argv[]);

	private:
		void handle_parsing_error();

		// returns the filename from a given full input path
		std::string get_filename(const std::string & input_path);
	};
    
}