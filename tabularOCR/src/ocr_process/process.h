#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "../preprocessing/preprocess.h"
#include <baseapi.h>
#include <renderer.h>
#include <tablefind.h>

namespace ocr
{
	void set_border(PIX* img, BOX *box, int r, int g, int b);

	Pix *matToPix(cv::Mat *mat);

	void process_image(std::pair<std::string, cv::Mat> & image);

}