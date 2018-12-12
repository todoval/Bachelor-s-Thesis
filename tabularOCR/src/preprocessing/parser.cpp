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

	bool process_binar_arg(std::string & arg, config & cfg)
	{
		std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);
		if (arg == "GLOBAL")
			cfg.bin_method = binarization_method::GLOBAL;
		else if (arg == "OTSU")
			cfg.bin_method = binarization_method::OTSU;
		else if (arg == "ADAPTIVEG")
			cfg.bin_method = binarization_method::ADAP_GAUS;
		else if (arg == "ADAPTIVEM")
			cfg.bin_method = binarization_method::ADAP_MEAN;
		else if (arg == "BERNSEN")
			cfg.bin_method = binarization_method::BERNSEN;
		else if (arg == "NIBLACK")
			cfg.bin_method = binarization_method::NIBLACK;
		else if (arg == "SAUVOLA")
			cfg.bin_method = binarization_method::SAUVOLA;
		else
			return false;
		return true;
	}

	bool process_greyscale_arg(std::string & arg, config & cfg)
	{
		std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);
		if (arg == "AVG")
			cfg.gs_method = greyscale_method::AVG;
		else if (arg == "LUMA")
			cfg.gs_method = greyscale_method::LUMA;
		else if (arg == "RED")
			cfg.gs_method = greyscale_method::SINGLE_R;
		else if (arg == "GREEN")
			cfg.gs_method = greyscale_method::SINGLE_G;
		else if (arg == "BLUE")
			cfg.gs_method = greyscale_method::SINGLE_B;
		else if (arg == "DESATURATE")
			cfg.gs_method = greyscale_method::DESATURATE;
		else
			return false;
		return true;
	}

	bool process_denoise_arg(std::string & arg, config & cfg)
	{
		std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);
		if (arg == "GAUSSIAN")
			cfg.noise_methods.push_back(denoise_method::GAUSSIAN);
		else if (arg == "MEAN")
			cfg.noise_methods.push_back(denoise_method::MEAN);
		else if (arg == "MEDIAN")
			cfg.noise_methods.push_back(denoise_method::MEDIAN);
		else if (arg == "NONLOCAL")
			cfg.noise_methods.push_back(denoise_method::NON_LOCAL);
		else if (arg == "WIENER")
			cfg.noise_methods.push_back(denoise_method::WIENER);
		else
			return false;
		return true;
	}

    config parse_args(int argc, char* argv[])
    {
        config result;
        size_t i = 1;
        while ( i < argc && argv[i][0]=='-')
        {
            //parse processing args
			std::string arg = argv[i];
			if (arg == "--scale" || arg == "-sc")
			{
				while (i < argc && arg[0] != '-' && process_scale_arg(arg, result))
					i++;
			}
			else if (arg == "--denoise" || arg == "-n")
			{

			}
			else if (arg == "--greyscale" || arg == "-g")
			{
				i++;
				arg = argv[i];
				while (i < argc && arg[0] != '-' && process_greyscale_arg(std::string(arg), result))
				{
					i++;
					arg = argv[i];
				}
			}
			else if (arg == "--binarize" || arg == "-b")
			{
				i++;
				arg = argv[i];
				while (i < argc && arg[0] != '-' && process_binar_arg(std::string(arg), result))
				{
					i++;
					arg = argv[i];
				}
			}
			else if (arg == "--deskew" || arg == "-sk")
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