#include "process.h"

namespace ocr
{

	std::string get_filename(const std::string & input_path)
	{
		int start = input_path.find_last_of("/\\");
		int end = input_path.find_last_of(".");
		return input_path.substr(start + 1, end - start - 1);
	}

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

	void box_merge_vertical(BOX* result, BOX* to_add)
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

	void box_merge_horizontal(BOX* result, BOX* to_add)
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

	int get_y_axis(std::vector<BOX*>  & input)
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
			|| abs(centre(first) - centre(second)) <= COL_THRESHOLD * 5)
			/*&& diff_h*/);
	}

	int get_width_of_col(BOX* first, BOX* second)
	{
		int first_part = abs(first->x - second->x);
		int sec_part = std::max(first->x + first->w, second->x + second->w) - std::max(first->x, second->x);
		return first_part + sec_part;
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
				for (int k = 0; k < first.size(); k++)
				{
					if (k == iter_one)
						continue;
					if (overlap(first[k], second[iter_two]))
						to_merge = false;
				}
				for (int k = 0; k < second.size(); k++)
				{
					if (k == iter_two)
						continue;
					if (overlap(second[k], first[iter_one]))
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
							page[i][j]->w = get_width_of_col(page[i][j], page[i + 1][sec_index]);
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
							page[i+1][j]->w = get_width_of_col(page[i+1][j], page[i][sec_index]);
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

	void initialize_font_cat(font_category & font_cat, int & ws, std::vector<int> & all_spaces, const std::vector<BOX*> & line)
	{
		double constant = font_cat.font / REF_FONT_SIZE;
		if (all_spaces.size() != 0)
			ws = get_whitespace(all_spaces, constant);
		font_cat.lines.push_back(line);
		font_cat.spaces.push_back(ws);
	}

	void delete_footer(std::vector<font_category> & font_cat)
	{
		// sort all line vectors by their y coordinate
		for (int i = 0; i < font_cat.size(); i++)
			std::sort(font_cat[i].lines.begin(), font_cat[i].lines.end(),
				[](std::vector<BOX*> & a, std::vector<BOX*> & b) { return a[0]->y < b[0]->y; });

		std::sort(font_cat.begin(), font_cat.end(),
			[](font_category & a, font_category & b) { return a.lines[a.lines.size() - 1][0]->y < b.lines[b.lines.size() - 1][0]->y; });

		// the font category with the lines at the end of the page is the last category in font_cat vector

		auto lines = font_cat.back().lines;

		// TO-DO - calculate footer threshold by something different than a constant

		int i = lines.size() - 1;
		while (i > 0)
		{
			// deal with multi-line footers
			int line_diff = get_y_axis(lines[i]) - get_y_axis(lines[i - 1]) - get_char_height(lines[i - 1]);
			if (line_diff > FOOTER_THRESHOLD)
				break;
			i--;
		}

		// get the last line before the footer, as it does not have to be in the same font category
		
		auto last_line = font_cat[0].lines.back();
		if (i > 0)
			last_line = lines[i-1];
		for (int j = 1; j < font_cat.size()-1; j++)
		{
			if (font_cat[j].lines.back()[0]->y > last_line[0]->y)
				last_line = font_cat[j].lines.back();
		}

		if ((get_y_axis(lines[i]) - get_y_axis(last_line) - get_char_height(last_line)) > FOOTER_THRESHOLD)
			font_cat.back().lines.erase(font_cat.back().lines.begin()+i);
	}

	void process_image(char* filename)
	{

		//Pix *img = pixRead("D:/bachelor_thesis/tabularOCR/test_images/img/23-1.jpg");

		Pix *img = pixRead(filename);
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
		// iterate over textlines and symbols to find all symbols in current textline
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
			if (one_line.size() > 0)
			{
				line_map.insert(std::pair<std::vector<BOX*>, int>(one_line, font));
				all_lines.push_back(one_line);
				one_line.clear();
			}
		}
		std::sort(fonts.begin(), fonts.end());

		std::vector<font_category> font_cat = get_font_categories(fonts);

		// determine font category and add all the necessary spaces
		for (auto line : line_map)
		{
			int ws = img->w;
			std::vector<int> all_spaces = get_spaces(line.first);
			int i = 0;
			while (i < font_cat.size() && line.second > font_cat[i].font)
				i++;
			if (i == 0)
				initialize_font_cat(font_cat[0], ws, all_spaces, line.first);
			else if (i == font_cat.size()
				|| font_cat[i].font - line.second > line.second - font_cat[i - 1].font)
				initialize_font_cat(font_cat[i-1], ws, all_spaces, line.first);
			else
				initialize_font_cat(font_cat[i], ws, all_spaces, line.first);
		}

		// determine whitespace for each category
			
		for (int i = 0; i < font_cat.size(); i++)
		{
			if (font_cat[i].lines.size() == 0 || font_cat[i].spaces.size() == 0)
			{
				font_cat.erase(font_cat.begin() + i);
				i--;
				continue;
			}
			font_cat[i].whitespace = most_common_number(font_cat[i].spaces);
			if (font_cat[i].whitespace == img->w || font_cat[i].whitespace == 0)
			{
				font_cat.erase(font_cat.begin() + i);
				i--;
			}
		}

		// happens with image 147 - TO DO - probably a weird determination of tesseract symbols
		if (font_cat.size() == 0)
		{
			std::cout << "An unknown error has occured. Sorry.";
			return;
		}


		/* 
		delete footer could probably be implemented before determination of font categories
		pros - simpler, less time consuming function, only one iteration
		cons - multiline footers
		*/
		delete_footer(font_cat);

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
						//set_border(img, column, 0, 255, 0);
						column = curr_words[j + 1];
					}
					else
						box_merge_horizontal(column, curr_words[j + 1]);
				}
				columns.push_back(column);
				//set_border(img, column, 0, 255, 0);
				if (columns.size() > 0)
					all_cols.push_back(columns);
			}
		}

		std::sort(all_cols.begin(), all_cols.end(),
			[](std::vector<BOX*> & a, std::vector<BOX*> & b) { 
			if (a[0]->y == b[0]->y && a.size() == b.size())
				return a[0]->x < b[0]->x;
			if (a[0]->y == b[0]->y)
				return a.size() < b.size();
			return a[0]->y < b[0]->y; 
		});

		// merge columns into tables

		merge_cols(all_cols);
		for (int i = 0; i < all_cols.size(); i++)
		{
			for (int j = 0; j < all_cols[i].size(); j++)
			{
				if (all_cols[i].size() > 1)
					set_border(img, all_cols[i][j], 0, 0, 255);
			}
		}

		std::string out = "results/" + get_filename(filename)+ ".png";

		char* path = &out[0u];
		pixWrite(path, img, IFF_PNG);
		api->End();
		pixDestroy(&img);
	}

}

/*
TO DO:
- determine footer threshold differently than by a constant
- single row tables -> delete
- preprocess multiple dots ... in an image -> delete
- allow minor mistakes in alingments
- full-page tables - whitespace problem, determine whitespace differently? 

*/