#include "parser.h"

namespace preprocessing
{
	config::config()
	{
		bin_method = binarization_method::NONE_B;
	}


	/* prepinace
		scaling: -sc, --scale, parametre: DPI, method;
		denoise: -n, --denoise, parametre:
		greyscale: -g, --greyscale, parametre:
		binarize: -b, --binarize, parametre:
		skew correction: -sk, --deskew, parametre:
		enhance: -e, --enhance,
	*/

	std::vector<file_info> config::parse_args(int argc, char * argv[])
	{
		std::vector<file_info> result;
		bool parse_files = false;
		auto curr_parsing = preprocess_method::NONE_PRE;
		for (size_t i = 0; i < argc; i++)
		{
			std::string arg = argv[i];

			// check whether the argument is not a switch argument
			if (arg[0] == '-')
			{
				if (arg == "--scale" || arg == "-sc")
				{
					curr_parsing = preprocess_method::SCALE;
					continue;
				}
				else if (arg == "--denoise" || arg == "-n")
				{
					curr_parsing = preprocess_method::DENOISE;
					continue;
				}
				else if (arg == "--greyscale" || arg == "-g")
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
					curr_parsing = preprocess_method::SCALE;
					continue;
				}
				else if (arg == "--enhance" || arg == "-e")
				{
					curr_parsing = preprocess_method::ENHANCE;
					continue;
				}
				else
					handle_parsing_error();
			}
			std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);
			bool parsing_args = true;
			switch (curr_parsing)
			{
			case (preprocess_method::BINARIZE):
				if (arg == "otsu")
					bin_method = binarization_method::OTSU;
				else if (arg == "sauvola")
					bin_method = binarization_method::SAUVOLA;
				else
					parse_files = true;
				break;
			case (preprocess_method::DENOISE):
				break;
			case (preprocess_method::ENHANCE):
				break;
			case (preprocess_method::GREYSCALE):
				break;
			case (preprocess_method::SCALE):
				break;
			case (preprocess_method::SKEW):
				break;
			case (preprocess_method::NONE_PRE):
				parsing_args = false;
				break;
			}
			if (parsing_args)
				continue;

			// parse files
			parse_files = true;
			std::ifstream input(arg);
			if (!input.good()) // check whether file exists
				handle_parsing_error();
			auto filename = get_filename(arg);
			auto img = pixRead(arg.c_str());
			if (img->d == 8)
				img = pixConvert8To32(img);
			Pix * copy;
			pixCopy(copy, img);
			result.push_back({ filename, img, copy });
		}
		return result;
	}

	void config::handle_parsing_error()
	{
		std::cout << "Error" << std::endl;
	}

	std::string config::get_filename(const std::string & input_path)
	{
		int start = input_path.find_last_of("/\\");
		int end = input_path.find_last_of(".");
		return input_path.substr(start + 1, end - start - 1);
	}



}