#include <memory>

#include <allheaders.h>

#include <baseapi.h>
#include <renderer.h>
#include <basedir.h>

#include "src/preprocessing/preprocess.h"

int main(int argc, char* argv[]) {

	preprocessing::config cfg = preprocessing::parse_argss(argc, argv);
	for (size_t i = 0; i < cfg.files.size(); i++)
	{

		preprocessing::preprocess_img(cfg.files[i]);
		preprocessing::grey_conversion(cfg.files[i], preprocessing::greyscale_method::SIMPLE);
		//std::string filename = preparation::get_filename(cfg.files[i]);
		cv::imwrite("E:/bachelor_thesis/tabularOCR/out.jpg", cfg.files[i]);
	}
	
	

	/*
	char* imageName = argv[3];
	cv::Mat image;
	cv::Mat dst_img;
	image = cv::imread(imageName, 1);
	cv::resize(image, dst_img, );
	//
	*/
    return 0;
}