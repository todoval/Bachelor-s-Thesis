#include "parser.h"

namespace preprocessing
{
	std::string get_filename(const std::string & input_path)
	{
		int start = input_path.find_last_of("/\\");
		int end = input_path.find_last_of(".");
		return input_path.substr(start + 1, end - start - 1);
	}

	bool is_number(const std::string& str)
	{
		std::string::const_iterator it = str.begin();
		while (it != str.end() && std::isdigit(*it)) ++it;
		return !str.empty() && it == str.end();
	}

	bool process_scale_arg(const std::string & arg, config & cfg)
	{
		if (is_number(arg))
		{
			cfg.sc_dpi = std::stoi(arg);
			return true;
		}
		return true;
		return false;
	}

	bool process_greyscale_arg(std::string & arg, config & cfg)
	{
		std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);
		if (arg == "SIMPLE")
		{
			cfg.gs_method = greyscale_method::SIMPLE;
			return true;
		}
		else if (arg == "LUMA")
		{
			cfg.gs_method = greyscale_method::LUMA;
			return true;
		}
		else if (arg == "RED")
		{
			cfg.gs_method = greyscale_method::SINGLE_R;
			return true;
		}
		else if (arg == "GREEN")
		{
			cfg.gs_method = greyscale_method::SINGLE_G;
			return true;
		}
		else if (arg == "BLUE")
		{
			cfg.gs_method = greyscale_method::SINGLE_B;
			return true;
		}
		else if (arg == "DESATURATE")
		{
			cfg.gs_method = greyscale_method::DESATURATE;
			return true;
		}
		else
			return false;
	}

    config parse_argss(int argc, char* argv[])
    {
        config result;
        size_t i = 1;
        while ( i < argc && argv[i][0]=='-')
        {
            //parse processing args
			if (argv[i] == "--scale" || argv[i] == "-sc")
			{
				while (i < argc && argv[i][0] != '-' && process_scale_arg(argv[i], result))
					i++;
			}
			else if (argv[i] == "--denoise" || argv[i] == "-n")
			{

			}
			else if (argv[i] == "--greyscale" || argv[i] == "-g")
			{
				while (i < argc && argv[i][0] != '-' && process_greyscale_arg(std::string(argv[i]), result))
					i++;
			}
			else if (argv[i] == "--binarize" || argv[i] == "-b")
			{

			}
			else if (argv[i] == "--deskew" || argv[i] == "-sk")
			{

			}
			else
			{
				// TO DO - program fail
				std::cout << "Unknown parameter " + std::string(argv[i]);
				return result;
			}
        }
        if (i == argc)
        {
			// TO DO - program fail
            std::cout << "No input files inserted";
            return result;
        }
		while (i < argc) // process all files
		{
			std::ifstream input(argv[i]);
			if (!input.good()) // check whether file exists
			{
				std::cout << "File " + std::string(argv[i]) + " does not exist" << std::endl;
				i++;
				continue;
			}
			cv::Mat image = cv::imread(argv[i], 1); // read a color image
			result.files.push_back(image);
			i++;
		}
		return result;
    }

	config::config()
	{
		sc_dpi = 300;

		gs_method = greyscale_method::LUMA;
	}

}