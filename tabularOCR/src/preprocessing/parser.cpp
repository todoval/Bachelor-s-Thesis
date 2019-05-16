#include "parser.h"

#include <algorithm>

using namespace tabular_ocr;

	config::config()
	{
		// default configuration
		bin_method = binarization_method::NONE_B;
		en_method = enhancement_method::NONE_E;
		deskew = false;
		img_output = false;
		json_output = false;
		gs_method = greyscale_method::NONE_G;
	}

	std::map<std::string, bool> config::parse_args(int argc, char * argv[])
	{
		std::map<std::string, bool> result;
		auto curr_parsing = preprocess_method::NONE_PRE;

		bool parsing_args = true;
		bool parse_files = false;

		// processing the help option
		if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help"))
		{
			output_help();
			exit(0);
		}
		for (int i = 1; i < argc; i++)
		{
			std::string arg = argv[i];

			// check whether the argument is not a switch argument
			if (!parse_files)
			{
				if (arg[0] == '-')
				{
					if (arg == "--greyscale" || arg == "-g")
					{
						curr_parsing = preprocess_method::GREYSCALE;
						continue;
					}
					else if (arg == "--binarize" || arg == "-b")
					{
						curr_parsing = preprocess_method::BINARIZE;
						continue;
					}
					else if (arg == "--deskew" || arg == "-sk")
					{
						deskew = true;
						continue;
					}
					else if (arg == "--enhance" || arg == "-e")
					{
						curr_parsing = preprocess_method::ENHANCE;
						continue;
					}
					else if (arg == "--preprocess" || arg == "-p")
					{
						// set preprocessing parameters to "ideal"
						en_method = enhancement_method::SIMPLE;
						continue;
					}
					else if (arg == "--output-json")
					{
						json_output = true;
						continue;
					}
					else if (arg == "--output-image")
					{
						img_output = true;
						continue;
					}
					else
					{
						std::cerr << "Invalid argument " << arg << std::endl;
						output_help();
						exit(1);
					}
				}
				auto old_arg = arg;
				std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);
				switch (curr_parsing)
				{
				case (preprocess_method::BINARIZE):
					if (arg == "OTSU")
						bin_method = binarization_method::OTSU;
					else if (arg == "SAUVOLA")
						bin_method = binarization_method::SAUVOLA;
					else
						parsing_args = false;
					break;
				case (preprocess_method::ENHANCE):
					if (arg == "SIMPLE")
						en_method = enhancement_method::SIMPLE;
					else if (arg == "GAMMA")
						en_method = enhancement_method::GAMMA;
					else if (arg == "EQUALIZATION")
						en_method = enhancement_method::HIST_EQUALIZATION;
					else
						parsing_args = false;
					break;
				case (preprocess_method::GREYSCALE):
					if (arg == "AVG")
						gs_method = greyscale_method::AVG;
					else if (arg == "MIN")
						gs_method = greyscale_method::MIN;
					else if (arg == "MAX")
						gs_method = greyscale_method::MAX;
					else if (arg == "LUMA")
						gs_method = greyscale_method::LUMA;
					else
						parsing_args = false;
					break;
				case (preprocess_method::NONE_PRE):
					parsing_args = false;
					break;
				}
				if (parsing_args)
					continue;
				arg = old_arg;
			}			

			// parse files

			parse_files = true;

			if (std::experimental::filesystem::is_directory(arg))
			{
				auto filenames = get_filenames_from_dir(arg);
				for (auto name : filenames)
				{
					if (std::experimental::filesystem::is_directory(name))
						continue;
					if (check_file(name))
						result[name] = true;
				}
			}
			else
			{
				if (check_file(arg))
					result[arg] = false;
			}
		}
		// if no output is specified, output both files
		if (!img_output && !json_output)
		{
			img_output = true;
			json_output = true;
		}

		return result;
	}

	std::string tabular_ocr::get_filename(const std::string & input_path)
	{
		int start = input_path.find_last_of("/\\");
		int end = input_path.find_last_of(".");
		return input_path.substr(start + 1, end - start - 1);
	}

	std::string tabular_ocr::get_filename_with_dir(const std::string & input_path)
	{
		int start = input_path.substr(0, input_path.find_last_of("/\\")).find_last_of("/\\");
		int end = input_path.find_last_of(".");
		return input_path.substr(start + 1, end - start - 1);
	}

	bool tabular_ocr::file_exists(const std::string & input_path)
	{
		std::ifstream infile(input_path);
		return infile.good();
	}

	bool tabular_ocr::is_file_format_valid(const std::string & input_path)
	{
		int delim = input_path.find_last_of('.');
		auto ext = input_path.substr(delim+1, input_path.size() - delim);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if (ext == "tif" || ext == "jpg" || ext == "png" || ext == "gif")
			return true;
		return false;
	}

	void tabular_ocr::create_results_subdirectory(const std::string & name)
	{
		int end = name.find_last_of("/ \\");
		int start = name.substr(0, name.find_last_of("/ \\")).find_last_of("/\\");
		std::string dir_name = name.substr(start + 1, end - start - 1);
		std::experimental::filesystem::create_directory("results/" + dir_name);
	}

	void tabular_ocr::save_img_result(const std::string & name, const image & img)
	{
		std::string out = "results/" + name + ".png";
		char* path = &out[0u];
		pixWrite(path, img.get(), IFF_PNG);
	}

	void tabular_ocr::save_json_result(const std::string & name, const json & json_form)
	{
		std::string out_name = "results/" + name + ".json";
		std::ofstream outfile(out_name);
		outfile << json_form.dump(3) << std::endl;
	}

	bool tabular_ocr::check_file(const std::string & name)
	{
		// check whether file exists
		if (!tabular_ocr::file_exists(name))
		{
			std::cerr << "Invalid argument: file " << name << " does not exist." << std::endl;
			output_help();
			exit(1);
		}
		// check for valid format
		if (!is_file_format_valid(name))
		{
			std::cerr << "Error at file " << name << ": invalid file format." << std::endl <<
				"Supported file formats are: .png, .jpg, .tif, .gif."<< std::endl;
			return false;
		}
		return true;
	}

	void tabular_ocr::output_help()
	{
		std::cerr << "Usage: tabularOCR [-options] (filenames | directory name)" << std::endl <<
			"where options include:" << std::endl <<
			"\t" << "(-e | --enhance) (SIMPLE | GAMMA | EQUALIZATION)" << std::endl <<
			"\t\t" << "enhance the contrast of the image before processing" << std::endl <<
			"\t" << "(-g | --greyscale) (AVG | MIN | MAX | LUMA)" << std::endl <<
			"\t\t" << "set the image mode to greyscale before processing" << std::endl <<
			"\t" << "(-b | --binarize) (OTSU | SAUVOLA)" << std::endl <<
			"\t\t" << "binarize image before processing" << std::endl <<
			"\t" << "-p | --preprocess" << std::endl <<
			"\t\t" << "preprocess image with the default preprocessing options before processing" << std::endl << 
			"\t" << "-sk | --deskew" << std::endl <<
			"\t\t" << "deskew image before processing" << std::endl <<
			"\t" << "--output-json" << std::endl <<
			"\t\t" << "output the result in a json file" << std::endl <<
			"\t" << "--output-image" << std::endl <<
			"\t\t" << "output the result in an png file as a bounding box around each cell" << std::endl;
	}

	std::vector<std::string> tabular_ocr::get_filenames_from_dir(const std::string & directory)
	{
		std::vector<std::string> result;
		std::experimental::filesystem::path path(directory);
		for (auto it : std::experimental::filesystem::directory_iterator(path))
			result.push_back(it.path().generic_string());
		return result;
	}

	file_info tabular_ocr::create_file_from_name(const std::string & name, bool is_dir)
	{
		// get the filename
		std::string filename;
		if (is_dir)
			filename = get_filename_with_dir(name);
		else
			filename = get_filename(name);
		// load the file into memory
		image img((struct Pix*)pixRead(name.c_str()));
		if (img->d == 8)
			img = image(pixConvert8To32(img.get()));
		image img_copy(pixCopy(NULL, img.get()));
		// create a copy of the file
		return file_info{filename, std::move(img), std::move(img_copy)};
	}
