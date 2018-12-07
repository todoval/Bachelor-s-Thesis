#include <string>
#include "../ocr_process/process.h"

// preprocessing class used for parsing arguments and conversion of files


namespace preparation
{
	/*
	get only the filename without extension and path
	*/
	std::string get_filename(std::string & input_path);

    /*
	convert all input files in the input_vector to a .tiff file,
	save the result back to vector (override input files)
	output files are saved to a temporary location
	*/

	/*
	parse the input arguments into a ocr::Process_info
	*/
    ocr::Process_info parse_args(int argc, char* argv[]);
    
}