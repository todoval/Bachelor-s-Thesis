#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctype.h>
#include <vector>
#include <map>
#include <experimental/filesystem>

#include <baseapi.h>
#include <renderer.h>
#include "allheaders.h"

namespace tabular_ocr
{

	/* prepinace
	scaling: -sc, --scale, parametre: DPI, method;
	denoise: -n, --denoise, parametre:
	greyscale: -g, --greyscale, parametre:
	binarize: -b, --binarize, parametre:
	skew correction: -sk, --deskew, parametre:
	enhance: -e, --enhance,
	*/

	enum preprocess_method { SCALE, DENOISE, GREYSCALE, BINARIZE, ENHANCE, NONE_PRE };

	enum scale_method { NONE_SC };

	enum greyscale_method { AVG, LUMA, MIN, MAX, NONE_G };

	enum binarization_method { OTSU, SAUVOLA, NONE_B };

	enum denoise_method { GAUSSIAN, MEAN, BILATERAL, MEDIAN, NON_LOCAL, NONE_N };

	enum enhancement_method { HIST_EQUALIZATION, SIMPLE, GAMMA, NONE_E };

	// a structure that is used for storing information about concrete images
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
		greyscale_method gs_method;
		binarization_method bin_method;
		enhancement_method en_method;
		bool deskew;

		std::map<std::string, bool> parse_args(int argc, char* argv[]);

	};

	// returns the file with initialized pix from the given name
	file_info create_file_from_name(const std::string & name, bool is_dir);

	// returns a vector of all names of files that are in the given directory
	std::vector<std::string> get_filenames_from_dir(const std::string& directory);

	// ends the whole program and throws a parsing error
	void handle_parsing_error();

	// returns a filename from a given full input path
	std::string get_filename(const std::string & input_path);

	// returns a filename with its parent directory from a given full input path
	std::string get_filename_with_dir(const std::string & input_path);

	// creates subdirectory of the results directory
	void create_results_subdirectory(const std::string & name);

	void save_result(file_info & file);
}