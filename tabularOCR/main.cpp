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
	

	/*
	preprocessing::config cfg = preprocessing::parse_args(argc, argv);
	for (auto &iter : cfg.files)
	{
		//std::string output_path = "E:/bachelor_thesis/tabularOCR/preprocessed" + iter.first + ".jpg";
		preprocessing::config img_cfg = cfg;
		cv::Mat img = iter.second;
		//preprocessing::preprocess_img(img, cfg);
		//cv::imwrite(output_path, img);
		//std::pair<std::string, cv::Mat> k = iter;
		//ocr::process_image(k);
	}
	*/
	std::experimental::filesystem::create_directory("results");

	// check for directory

	ocr::page page ("D:/bachelor_thesis/tabularOCR/test_images/img/5-1.jpg");
	page.process_image();

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
	}

	/*
	TO DO
	- scale the image - zistit ako, co, preco (??), ideal 300 dpi
	- zistit ci je image scanned alebo nie
	- ak je, spracovavat ho inak, ak nie je, vykaslat sa na skew, denoise etc


	*/
    return 0;
}
