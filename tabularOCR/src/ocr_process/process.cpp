#include "process.h"

namespace ocr
{
	void set_border(PIX* img, BOX *box, int r, int g, int b)
	{
		for (int i = 0; i < box->w; i++)
		{
			pixSetRGBPixel(img, box->x + i, box->y, r, g, b);
			pixSetRGBPixel(img, box->x + i, box->y + box->h, r, g, b);
		}
		for (int i = 0; i < box->h; i++)
		{
			pixSetRGBPixel(img, box->x, box->y + i, r, g, b);
			pixSetRGBPixel(img, box->x + box->w, box->y + i, r, g, b);
		}
	}

	Pix *matToPix(cv::Mat *mat)
	{
		Pix *pixd = pixCreate(mat->size().width, mat->size().height, 32);
		for (int y = 0; y < mat->rows; y++) {
			for (int x = 0; x < mat->cols; x++) {
				pixSetPixel(pixd, x, y, (l_uint32)mat->at<uchar>(y, x));
			}
		}
		return pixd;
	}

	bool is_symbol_in_textline(BOX* symbol, BOX* textline)
	{
		return (symbol->x <= textline->x + textline->w 
			&& symbol->x >= textline->x 
			&& symbol->y >= textline->y 
			&& symbol->y <= textline->y + textline->h);
	}

	std::vector<int> get_spaces(const std::vector<BOX*> & symbols)
	{
		std::vector <int> result;
		// iterate over all symbols and return the whitespaces between them
		for (int i = 0; i < symbols.size() - 1; i++)
		{
			BOX* first = symbols[i];
			BOX* second = symbols[i + 1];
			int space = second->x - first->x - first->w;
			result.push_back(space);
		}
		return result;
	}

	int get_whitespace(std::vector<int> & all_spaces, double constant)
	{
		std::sort(all_spaces.begin(), all_spaces.end());
		// heuristical estimation of the space
		int i = 0;
		while (i < all_spaces.size() && all_spaces[i] < constant)
			i++;
		while (i < all_spaces.size() - 1)
		{
			double multi_factor = get_multi_factor(all_spaces[i], constant);
			if (all_spaces[i + 1] >= multi_factor * all_spaces[i])
			{
				i++;
				break;
			}
			i++;
		}
		int ws = 0;
		if (i < all_spaces.size())
			ws = all_spaces[i];
		return ws;
	}

	void box_merge_vertical(BOX* & result, BOX* & to_add)
	{
		if (result == nullptr)
			result = to_add;
		else
		{
			result->w = std::max(result->w, to_add->w);
			result->x = std::min(result->x, to_add->x);
			result->y = std::min(result->y, to_add->y);
			result->h = to_add->h + to_add->y - result->y;
		}
	}

	void box_merge_horizontal(BOX* & result, BOX* & to_add)
	{
		if (result == nullptr)
			result = to_add;
		else
		{
			result->w = to_add->w + to_add->x - result->x;
			result->h = std::max(result->h, to_add->h);
			result->y = std::min(result->y, to_add->y);
		}
	}

	std::vector<BOX*> merge_into_words(std::vector<BOX*> & symbols, int whitespace)
	{
		std::sort(symbols.begin(), symbols.end(),
			[](BOX* & a, BOX* & b) { return a->x < b->x; });

		std::vector<BOX*> result = {};
		BOX* word = symbols[0];
		for (size_t i = 0; i < symbols.size() - 1; i++)
		{
			if (symbols[i]->x + symbols[i]->w + whitespace > symbols[i + 1]->x)
				box_merge_horizontal(word, symbols[i + 1]);
			else
			{
				result.push_back(word);
				word = symbols[i + 1];
			}
		}
		result.push_back(word);
		return result;
	}

	int get_char_height(std::vector<BOX*> & symbols)
	{
		int height = 0;
		for (int i = 0; i < symbols.size(); i++)
			height = std::max(height, symbols[i]->h);
		return height;
	}

	double get_multi_factor(int space_width, double constant)
	{
		double x = space_width / constant;
		if (x >= 4)
			return 1.5;
		if (x < 4 && x >= 3)
			return ((4 - x)* 0.1 + 1.5);
		if (x < 3 && x >= 2)
			return ((3 - x) * 0.4 + 1.6);
		if (x < 2 && x >= 1)
			return (4 - x);
		return 0;
	}

	int most_common_number(std::vector<int> & numbers)
	{
		std::sort(numbers.begin(), numbers.end());

		int res_no = 0;
		int curr_no = numbers[0];
		int curr_count = 1;
		int res_count = 0;
		for (size_t i = 1; i < numbers.size(); i++)
		{
			if (numbers[i] == curr_no)
				curr_count++;
			else
			{
				if (res_count < curr_count)
				{
					res_count = curr_count;
					res_no = curr_no;
				}
				curr_no = numbers[i];
				curr_count = 1;
			}
		}
		if (res_count < curr_count)
			res_no = curr_no;
		return res_no;
	}

	std::vector<font_category> get_font_categories(std::vector<int> & fonts)
	{
		fonts.erase(std::unique(fonts.begin(), fonts.end()), fonts.end());
		int difference = 4;
		int k = 1;
		std::vector<font_category> categories = {};
		int first_val = fonts[0];
		std::vector<int> one_cat = { first_val };
		int i = 1;

		while (i < fonts.size())
		{
			while (i < fonts.size() && fonts[i] < 10 * k && fonts[i - 1] + difference > fonts[i] && first_val + difference >= fonts[i])
			{
				one_cat.push_back(fonts[i]);
				i++;
			}
			if (i == fonts.size())
				break;
			else if (fonts[i] >= 10 * k)
			{
				k++;
				difference++;
			}
			else
			{
				int sum = 0;
				for (size_t j = 0; j < one_cat.size(); j++) // mby median?
					sum += one_cat[j];
				font_category new_cat;
				new_cat.font = sum / one_cat.size();
				new_cat.lines = {};
				new_cat.whitespace = 0;
				new_cat.spaces = {};
				categories.push_back(new_cat);
				one_cat.clear();
				first_val = fonts[i];
				one_cat.push_back(first_val);
				i++;
			}
		}
		int sum = 0;
		for (size_t j = 0; j < one_cat.size(); j++) //mby median?
			sum += one_cat[j];
		font_category new_cat;
		new_cat.font = sum / one_cat.size();
		new_cat.lines = {};
		new_cat.whitespace = 0;
		new_cat.spaces = {};
		categories.push_back(new_cat);
		return categories;
	}

	int centre(BOX* box)
	{
		return (box->x + box->w / 2);
	}

	int get_y_axis(std::vector<BOX*> input)
	{
		int y = input[0]->y;
		for (int i = 1; i < input.size(); i++)
			y = std::min(y, input[i]->y);
		return y;
	}

	bool overlap(BOX* first, BOX* second)
	{
		return ((second->x < first->x + first->w && second->x > first->x)
				|| (first->x < second->x + second->w && first->x > second->x));
	}

	bool are_in_same_col(BOX* first, BOX* second)
	{
		bool diff_h; 
		if (first->y < second->y)
			diff_h = second->y - first->y - first->h < std::min(first->h, second->h);
		else
			diff_h = first->y - second->y - second->h < std::min(first->h, second->h);
		return ((abs(first->x - second->x) <= COL_THRESHOLD
			|| abs(first->x + first->w - (second->x + second->w)) <= COL_THRESHOLD
			|| abs(centre(first) - centre(second)) <= COL_THRESHOLD)
			/*&& diff_h*/);
	}

	void merge_cols(std::vector<std::vector<BOX*>> & page)
	{
		int i = 0;
		bool find_cols = false;
		while (i < page.size() - 1)
		{
			find_cols = true;
			// first vector is the one that has the most elements
			std::vector<BOX*> first = page[i + 1];
			std::vector<BOX*> second = page[i];
			if (page[i].size() > page[i + 1].size())
			{
				first = page[i];
				second = page[i + 1];
			}
			// map for saving all the indexes of columns for further merging
			std::map<int, int> no_of_cols;
			int iter_one = 0;
			int iter_two = 0;
			while (iter_one < first.size() && iter_two < second.size())
			{
				while (iter_one < first.size() && iter_two < second.size() && !are_in_same_col(first[iter_one], second[iter_two]))
					iter_one++;
				if (first.size() == iter_one)
					break;

				// check whether the chosen columns don't overlap with anything else

				bool to_merge = true;
				for (int i = 0; i < first.size(); i++)
				{
					if (i == iter_one)
						continue;
					if (overlap(first[i], second[iter_two]))
						to_merge = false;
				}
				for (int i = 0; i < second.size(); i++)
				{
					if (i == iter_two)
						continue;
					if (overlap(second[i], first[iter_one]))
						to_merge = false;
				}

				if (to_merge)
					no_of_cols.insert(std::pair<int, int>(iter_one, iter_two));
				iter_two++;
				iter_one++;
			}
			if (no_of_cols.size() > 0)
			{
				// merge these two lines into columns
				int first_h = get_char_height(page[i]);
				int sec_h = get_char_height(page[i + 1]);
				int first_y = get_y_axis(page[i]);
				int sec_y = get_y_axis(page[i + 1]);

				// calculate the y axis and height of all the boxes

				int y = first_y;
				int height = sec_h + sec_y - first_y;

				if (first == page[i])
				{
					for (int j = 0; j < page[i].size(); j++)
					{
						if (no_of_cols.find(j) != no_of_cols.end())
						{
							int sec_index = no_of_cols[j];
							page[i][j]->x = std::min(page[i][j]->x, page[i + 1][sec_index]->x);
							page[i][j]->w = std::max(page[i][j]->w, page[i + 1][sec_index]->w);
						}
						page[i][j]->h = height;
						page[i][j]->y = y;
					}

					// erase the second vector
					page.erase(page.begin() + i + 1);
				}
				else
				{
					for (int j = 0; j < page[i+1].size(); j++)
					{
						if (no_of_cols.find(j) != no_of_cols.end())
						{
							int sec_index = no_of_cols[j];
							page[i+1][j]->x = std::min(page[i][sec_index]->x, page[i + 1][j]->x);
							page[i+1][j]->w = std::max(page[i][sec_index]->w, page[i + 1][j]->w);
						}
						page[i+1][j]->h = height;
						page[i+1][j]->y = y;
					}

					// erase the first vector
					page.erase(page.begin() + i);
				}


			}
			else
				i++;
		}
	}

	bool cols_to_tables(std::vector<BOX*> & first, std::vector<BOX*> & second)
	{
 
		if (first.size() == 1 && second.size() == 1)
			return false;
		if (first.size() == second.size())
		{

			return true;
		}
		return false;
	}

	void initialize_font_cat(font_category & font_cat, int & ws, std::vector<int> & all_spaces, const std::vector<BOX*> & line)
	{
		double constant = font_cat.font / REF_FONT_SIZE;
		if (all_spaces.size() != 0)
			ws = get_whitespace(all_spaces, constant);
		font_cat.lines.push_back(line);
		font_cat.spaces.push_back(ws);
	}

	void process_image()
	{
		Pix *img = pixRead("E:/bachelor_thesis/tabularOCR/test_images/img/83-1.jpg");
		if (img->d == 8)
			img = pixConvert8To32(img);

		// itialize tesseract api without the use of LSTM
		tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
		if (api->Init(NULL, "eng", tesseract::OcrEngineMode::OEM_TESSERACT_ONLY))
		{
			fprintf(stderr, "Could not initialize tesseract.\n");
			exit(1);
		}

		api->SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
		api->SetImage(img);
		api->SetVariable("textord_tabfind_find_tables", "true");
		api->SetVariable("textord_tablefind_recognize_tables", "true");
		api->Recognize(0);

		// array of symbols that exist in the image
		Boxa * symbol_arr = api->GetComponentImages(tesseract::RIL_SYMBOL, false, NULL, NULL);
		// array of textlines that exist in the image
		Boxa * textline_arr = api->GetComponentImages(tesseract::RIL_TEXTLINE, false, NULL, NULL);

		std::vector <std::vector<BOX*>> all_lines = {};
		std::vector<int> fonts = {};
		// mapping line -> font category
		std::map <std::vector<BOX*>, int> line_map;
		/*
		iterate over textlines and symbols to find all symbols in current textline
		*/
		std::vector<BOX*> one_line = {};
		for (size_t i = 0; i < textline_arr->n; i++)
		{
			BOX* textline = boxaGetBox(textline_arr, i, L_CLONE);
			// iterate over all symbols and add into vector those that are in the current textline
			for (size_t j = 0; j < symbol_arr->n; j++)
			{
				BOX* symbol = boxaGetBox(symbol_arr, j, L_CLONE);
				if (is_symbol_in_textline(symbol, textline))
					one_line.push_back(symbol);
			}
			int font = get_char_height(one_line);
			fonts.push_back(font);
			line_map.insert(std::pair<std::vector<BOX*>, int>(one_line, font));
			all_lines.push_back(one_line);
			one_line.clear();
		}
		std::sort(fonts.begin(), fonts.end());

		std::vector<font_category> font_cat = get_font_categories(fonts);

		std::map<std::vector<BOX*>, int>::iterator it;


		// determine font category and add all the necessary spaces
		for (auto line : line_map)
		{
			for (size_t i = 0; i < line.first.size(); i++)
			{
				//std::cout << line.first[i]->x << ":";
			}
			//std::cout << std::endl;

			int ws = img->w;
			std::vector<int> all_spaces = get_spaces(line.first);
			int i = 0;
			while (i < font_cat.size() && line.second > font_cat[i].font)
				i++;
			if (i == font_cat.size())
				initialize_font_cat(font_cat[i-1], ws, all_spaces, line.first);
			else if (i == 0)
				initialize_font_cat(font_cat[0], ws, all_spaces, line.first);
			else if (font_cat[i].font - line.second > line.second - font_cat[i - 1].font)
				initialize_font_cat(font_cat[i - 1], ws, all_spaces, line.first);
			else
				initialize_font_cat(font_cat[i], ws, all_spaces, line.first);
		}

		// determine whitespace for each category
		for (int i = 0; i < font_cat.size(); i++)
			font_cat[i].whitespace = most_common_number(font_cat[i].spaces);
		for (int i = 0; i < font_cat.size(); i++)
			if (font_cat[i].whitespace == img->w)
			{
				font_cat.erase(font_cat.begin() + i);
				i--;
			}

		// merge into words and columns
		std::vector<std::vector<BOX*>> all_cols = {};
		for (size_t i = 0; i < font_cat.size(); i++)
		{
			for (size_t j = 0; j < font_cat[i].lines.size(); j++)
			{
				std::vector<BOX*> curr_words = merge_into_words(font_cat[i].lines[j], font_cat[i].whitespace);
				for (size_t k = 0; k < curr_words.size(); k++)
				{
					// border around words
					//set_border(img, curr_words[k], 255, 0, 0);
					//std::cout << curr_words[k]->x << ":" << curr_words[k]->y << ":" << curr_words[k]->w << ":"
						//<< curr_words[k]->h << std::endl;
				}

				// determine whether the current line has columns or not

				std::vector<int> word_gaps = get_spaces(curr_words);
				std::vector<BOX*> columns = {};
				BOX* column = curr_words[0];
				for (size_t j = 0; j < word_gaps.size(); j++)
				{
					if (word_gaps[j] >= 3 * font_cat[i].whitespace)
					{
						columns.push_back(column);
						set_border(img, column, 0, 255, 0);
						column = curr_words[j + 1];
					}
					else
						box_merge_horizontal(column, curr_words[j + 1]);
				}
				columns.push_back(column);
				set_border(img, column, 0, 255, 0);
				if (columns.size() > 0)
					all_cols.push_back(columns);
			}
		}

		std::sort(all_cols.begin(), all_cols.end(),
			[](std::vector<BOX*> & a, std::vector<BOX*> & b) { return a[0]->y < b[0]->y; });

		// merge columns into tables

		merge_cols(all_cols);
		for (int i = 0; i < all_cols.size(); i++)
		{
			for (int j = 0; j < all_cols[i].size(); j++)
			{
				//if (all_cols[i].size() > 1)
					//set_border(img, all_cols[i][j], 0, 0, 255);
			}
		}

		std::string out = "E:/bachelor_thesis/tabularOCR/out83.jpg"; 
		char* path = &out[0u];
		pixWrite(path, img, IFF_PNG);
		api->End();
		pixDestroy(&img);

		// Get OCR result
		//char* outText = api->GetUTF8Text();
		//delete[] outText;
	}

}