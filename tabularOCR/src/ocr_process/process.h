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

	const size_t MAX_CHARS_IN_WORD = 15;

	const double REF_FONT_SIZE = 20;

	// sets border around the box to a given color
	void set_border(PIX* img, BOX *box, int r, int g, int b);

	Pix *matToPix(cv::Mat *mat);

	void process_image();

	bool is_symbol_in_textline(BOX* symbol, BOX* textline);
	
	// returns the whitespace between words in textline
	int get_whitespace(std::vector<BOX*> & symbols);

	std::vector<BOX*> merge_into_words(std::vector<BOX*> & symbols, int whitespace);

	int get_avg_char_width(std::vector<BOX*> & symbols);

	double get_multi_factor(int space_width, double constant);
}