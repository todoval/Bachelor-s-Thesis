#include "parser.h"

#include <algorithm>

using namespace tabular_ocr;

	config::config()
	{
		// default configuration
		bin_method = binarization_method::NONE_B;
		en_method = enhancement_method::NONE_E;
		deskew = false;
		gs_method = greyscale_method::NONE_G;
	}

	std::map<std::string, bool> config::parse_args(int argc, char * argv[])
	{
		std::map<std::string, bool> result;
		auto curr_parsing = preprocess_method::NONE_PRE;
		for (size_t i = 1; i < argc; i++)
		{
			std::string arg = argv[i];

			// check whether the argument is not a switch argument
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
					//gs_method = greyscale_method::MAX;
					en_method = enhancement_method::SIMPLE;
					continue;
				}
				else
					handle_parsing_error();
			}
			auto old_arg = arg;
			std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);
			bool parsing_args = true;
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

			// parse files

			arg = old_arg;

			if (std::experimental::filesystem::is_directory(arg))
			{
				auto filenames = get_filenames_from_dir(arg);
				for (auto name : filenames)
					result[name] = true;
			}
			else
				result[arg] = false;
		}
		return result;
	}

	void tabular_ocr::handle_parsing_error()
	{
		std::cout << "Error" << std::endl;
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

	void tabular_ocr::create_results_subdirectory(const std::string & name)
	{
		int end = name.find_last_of("/ \\");
		int start = name.substr(0, name.find_last_of("/ \\")).find_last_of("/\\");
		std::string dir_name = name.substr(start + 1, end - start - 1);
		std::experimental::filesystem::create_directory("results/" + dir_name);
	}

	void tabular_ocr::save_result(const std::string & name, const image & img)
	{
		std::string out = "results/" + name + ".png";
		char* path = &out[0u];
		pixWrite(path, img.get(), IFF_PNG);
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
		std::ifstream input(name);
		// check whether file exists
		if (!input.good())
			handle_parsing_error();
		input.close();
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
