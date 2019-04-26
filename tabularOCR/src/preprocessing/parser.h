#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <experimental/filesystem>

#ifdef _WIN32

#include <baseapi.h>
#include "allheaders.h"

#else

#include <tesseract/baseapi.h>
#include "leptonica/allheaders.h"

#endif // _WIN32

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

	typedef std::unique_ptr<Pix> image;

	enum preprocess_method { GREYSCALE, BINARIZE, ENHANCE, NONE_PRE };

	enum greyscale_method { AVG, LUMA, MIN, MAX, NONE_G };

	enum binarization_method { OTSU, SAUVOLA, NONE_B };

	enum enhancement_method { HIST_EQUALIZATION, SIMPLE, GAMMA, NONE_E };

	// a structure that is used for storing information about concrete images
	struct file_info
	{
		std::string name;
		image old;
		image preprocessed;
	};

	class config
	{
	public:
		config();

		// scale parameters
		int sc_dpi;
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

	void save_result(const std::string & name, const image & img);
}