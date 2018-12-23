#include <memory>

#include <baseapi.h>
#include <renderer.h>

#include "src/preprocessing/preprocess.h"
#include "src/ocr_process/process.h"

int main(int argc, char* argv[]) {
	/*preprocessing::config cfg = preprocessing::parse_args(argc, argv);
	for (auto &iter : cfg.files)
	{
		//std::string output_path = "E:/bachelor_thesis/tabularOCR/preprocessed" + iter.first + ".jpg";
		preprocessing::config img_cfg = cfg;
		cv::Mat img = iter.second;
		//preprocessing::preprocess_img(img, cfg);
		//cv::imwrite(output_path, img);
		std::pair<std::string, cv::Mat> k = iter;*/
		//ocr::process_image(k);
	//}
	
	ocr::process_image();



	/*
	TO DO
	- scale the image - zistit ako, co, preco (??), ideal 300 dpi
	- zistit ci je image scanned alebo nie
	- ak je, spracovavat ho inak, ak nie je, vykaslat sa na skew, denoise etc


	*/
    return 0;
}
