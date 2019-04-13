#include <memory>
#include <experimental/filesystem>

#include "src/preprocessing/preprocess.h"
#include "src/ocr_process/process.h"

std::vector<std::string> get_filenames(const std::string& directory) {
	std::vector<std::string> retVal;
	std::experimental::filesystem::path path(directory);
	for (auto it : std::experimental::filesystem::directory_iterator(path))
	{
		retVal.push_back(it.path().generic_string());
	}
	return retVal;
}

int main(int argc, char* argv[]) {
	
	preprocessing::config cfg;
	auto files = cfg.parse_args(argc, argv);

	preprocessing::preprocess_files(files, cfg);

	std::experimental::filesystem::create_directory("results");

	for (auto file : files)
	{
		ocr::page page(file);
		page.process_image();

	}

	// check for directory

//	ocr::page page ("D:/bachelor_thesis/tabularOCR/test_images/to_test/2-1.jpg");
	//page.process_image();

	/*
	std::vector<std::string> inputs;
	if (argc == 2 && std::experimental::filesystem::is_directory(argv[1]))
	{
		inputs = get_filenames(argv[1]);
		int i = 0;
		while (i < inputs.size())
		{
			char *cstr = &inputs[i][0u];
			ocr::page page(cstr);
			std::cout << i << ":";
			std::cout << cstr << std::endl;
			page.process_image();

			i++;
		}
	}
	else
	{
		int i = 1;
		while (i < argc)
		{
			ocr::page page (argv[i]);
			page.process_image();
			std::cout << i << ":";
			i++;
		}
	}*/

    return 0;
}
