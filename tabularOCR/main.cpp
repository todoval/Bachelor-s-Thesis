#include <memory>

#include <allheaders.h>

#include <baseapi.h>
#include <renderer.h>

#include "src/preprocessing/preprocess.h"

int main(int argc, char* argv[]) {

	preprocessing::config cfg = preprocessing::parse_args(argc, argv);
	for (std::map<std::string, cv::Mat>::iterator iter = cfg.files.begin(); iter != cfg.files.end(); ++iter)
	{
		std::string output_path = "E:/bachelor_thesis/tabularOCR/s" + iter->first + ".jpg";
		preprocessing::config img_cfg = cfg;
		cv::Mat img = iter->second;
		std::cout << img.size().width;
		preprocessing::preprocess_img(img, cfg);
		cv::imwrite(output_path, img);
	}
	




	/*
	TO DO
	- scale the image - zistit ako, co, preco (??), ideal 300 dpi
	- zistit ci je image scanned alebo nie
	- ak je, spracovavat ho inak, ak nie je, vykaslat sa na skew, denoise etc


	*/
    return 0;
}
