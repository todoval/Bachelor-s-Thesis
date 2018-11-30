#include <string>
#include <Magick++.h>
#include "../ocr_process/process.h"

using namespace Magick;

// preprocessing class used for parsing arguments and conversion of files


namespace preparation
{
    // convert all input images to a .tiff files
    void convert_to_tiff(std::vector<Image> & input_vector);

    ocr::Process_info parse_args(int argc, char* argv[]);
    
}