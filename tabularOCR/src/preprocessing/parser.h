#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <iomanip> 
#include <experimental/filesystem>

#ifdef _WIN32

#include <baseapi.h>
#include "allheaders.h"

#else

#include <tesseract/baseapi.h>
#include "leptonica/allheaders.h"

#endif // _WIN32

#include <json.hpp>
using json = nlohmann::json;

namespace tabular_ocr
{
	struct pixDestroy_wrap {
		void operator()(struct Pix* p) { pixDestroy(&p); }
	};

	using image = std::unique_ptr<struct Pix, pixDestroy_wrap>;

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

		greyscale_method gs_method;
		binarization_method bin_method;
		enhancement_method en_method;
		bool deskew;

		bool json_output;
		bool img_output;

		std::map<std::string, bool> parse_args(int argc, char* argv[]);
	};

	// returns the file with initialized pix from the given name
	file_info create_file_from_name(const std::string & name, bool is_dir);

	// returns a vector of all names of files that are in the given directory
	std::vector<std::string> get_filenames_from_dir(const std::string& directory);

	// returns a filename from a given full input path
	std::string get_filename(const std::string & input_path);

	// returns a filename with its parent directory from a given full input path
	std::string get_filename_with_dir(const std::string & input_path);

	// returns true if a file on the given input path exists
	bool file_exists(const std::string & input_path);

	// returns true if the format is one of the formats recognized by the Tesseract engine
	bool is_file_format_valid(const std::string & input_path);

	// creates subdirectory of the results directory
	void create_results_subdirectory(const std::string & name);

	void save_img_result(const std::string & name, const image & img);

	void save_json_result(const std::string & name, const json & json_form);

	// checks whether the file is valid and should be processed
	bool check_file(const std::string & name);

	// output the -help and ends the whole program
	void output_help();
}
