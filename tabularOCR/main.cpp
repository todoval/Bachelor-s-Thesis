#include "src/preprocessing/preprocess.h"
#include "src/ocr_process/process.h"

using namespace tabular_ocr;

int main(int argc, char* argv[]) {

	// create default configuration
	config cfg;
		
	// read all filenames that should be processed and parse configuration arguments
	
	auto filenames = cfg.parse_args(argc, argv);
		
	std::experimental::filesystem::create_directory("results");

	for (auto name : filenames)
	{
		// if the file was a directory, create a subdirectory
		if (name.second)
			create_results_subdirectory(name.first);

		// initialize images from given input files
		file_info file = create_file_from_name(name.first, name.second);

		preprocessing::preprocessor preprocessor(std::move(file.preprocessed), cfg);
		preprocessor.preprocess_file();
		file.preprocessed = std::move(preprocessor.img);

		ocr::page page(file);
		page.process_image();

		// saves the results given the output parameters

		if (cfg.img_output)
		{
			page.set_table_borders();
			save_img_result(file.name, page.img_old);
		}
		if (cfg.json_output)
		{
			json json_form = page.to_json();
			save_json_result(file.name, json_form);
		}

		std::cout << "File \"" + name.first + "\" has been processed." << std::endl;
	}
 	return 0;
}
