#include <string>
#include <iostream>
#include <fstream>
#include "parser.h"


namespace preparation
{

	std::string get_filename(std::string & input_path)
	{
		int start = input_path.find_last_of("/\\");
		int end = input_path.find_last_of(".");
		return input_path.substr(start + 1, end-start-1);
	}

    ocr::Process_info parse_args(int argc, char* argv[])
    {
        ocr::Process_info result;
        size_t i = 1;
        while ( i < argc && argv[i][0]=='-')
        {
            //parse processing args
            i++;
        }
        if (i == argc)
        {
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
			i++;
		}
		return result;
    }

}