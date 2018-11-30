#include <string>
#include <iostream>
#include "parser.h"
#include <vector>

using namespace Magick;

namespace preparation
{

    void convert_to_tiff(std::vector<Image> & input_vector)
    {
        try
        {
            for (size_t i = 0; i < input_vector.size(); i++)
            {
                input_vector[i].write("out.tiff");
            }
        }
        catch (Magick::WarningCoder &e)
        {
            std::cout << "Warning " << e.what() << std::endl;
        }
    }

    ocr::Process_info parse_args(int argc, char* argv[])
    {
        ocr::Process_info result;
        size_t i = 1;
        while ( i < argc && argv[i].at[0]=="-")
        {
            //parse processing args
            i++;
        }
        if (i == argc)
        {
            std::cout << "No input files inserted";
            return result;
        }


    }


}