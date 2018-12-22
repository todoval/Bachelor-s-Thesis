#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "../preprocessing/preprocess.h"
#include <baseapi.h>
#include <renderer.h>
#include <tablefind.h>
#include <algorithm> 

namespace ocr
{

	const size_t THRESHOLD = 10;

	void set_border(PIX* img, BOX *box, int r, int g, int b);

	Pix *matToPix(cv::Mat *mat);

	bool add_box(Box & result, Box &to_add);

	void process_image(std::pair<std::string, cv::Mat> & image);

	bool is_symbol_in_textline(BOX* symbol, BOX* textline);

	int get_whitespace(std::vector<BOX*> & symbols);

	std::vector<BOX*> merge_into_words(std::vector<BOX*> & symbols, int whitespace);
}