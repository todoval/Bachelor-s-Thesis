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

	const size_t COL_THRESHOLD = 10;

	const double REF_FONT_SIZE = 19;

	struct font_category
	{
	public:
		int font;
		int whitespace;
		std::vector<std::vector<BOX*>> lines;
		std::vector<int> spaces;
	};

	/*
	struct cell
	{
	public:
		BOX* box;
		std::string text;
	};

	struct table
	{
	public:
		// representation of the table by rows, each row represented by a vector of cells
		std::vector<std::vector<cell>> table_mat;
		size_t row_size;
		size_t col_size;
	};*/


	// sets border around the box to a given color
	void set_border(PIX* img, BOX *box, int r, int g, int b);

	Pix *matToPix(cv::Mat *mat);

	void process_image();

	bool is_symbol_in_textline(BOX* symbol, BOX* textline);
	
	std::vector<int> get_spaces(const std::vector<BOX*> & symbols);

	// returns the whitespace between words in textline
	int get_whitespace(std::vector<int> & all_spaces, double constant);

	void box_merge_horizontal(BOX* & result, BOX* & to_add);

	std::vector<BOX*> merge_into_words(std::vector<BOX*> & symbols, int whitespace);

	int get_char_height(std::vector<BOX*> & symbols);

	std::vector<font_category> get_font_categories(std::vector<int> & fonts);

	double get_multi_factor(int space_width, double constant);

	int most_common_number(std::vector<int> & numbers);

	void box_merge_vertical(BOX* & result, BOX* & to_add);

	bool cols_to_tables(std::vector<BOX*> & first, std::vector<BOX*> & second);

	void merge_cols(std::vector<std::vector<BOX*>> & page);

	bool are_in_same_col(BOX* first, BOX* second);

	void initialize_font_cat(font_category & font_cat, int & ws, std::vector<int> & all_spaces, const std::vector<BOX*> & line);

	bool overlap(BOX* first, BOX* second);

	int centre(BOX* box);

}