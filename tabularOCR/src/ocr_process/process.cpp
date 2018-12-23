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
		for (int i = 0; i < all_spaces.size(); i++)
			std::cout << all_spaces[i] << ":";
		std::cout << "RESULT:" << ws << std::endl;
		return ws;
	}

	void merge_boxes(BOX* & result, BOX* & to_add)
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
		std::vector<BOX*> result = {};
		BOX* word = symbols[0]; 
		for (size_t i = 0; i < symbols.size() - 1; i++)
		{	
			if (symbols[i]->x + symbols[i]->w + whitespace > symbols[i + 1]->x)
				merge_boxes(word, symbols[i + 1]);
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
		int sum = 0;
		for (int i = 0; i < symbols.size(); i++)
			sum = std::max(sum, symbols[i]->h);
		return sum;
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

	std::vector<font_category> get_font_categories(std::vector<int> & fonts)
	{
		fonts.erase(std::unique(fonts.begin(), fonts.end()), fonts.end());
		int difference = 4;
		int k = 1;
		std::vector<font_category> categories = {};
		int first_val = fonts[0];
		std::vector<int> one_cat = {first_val};
		int i = 1;

		while (i < fonts.size())
		{
			while (i < fonts.size() && fonts[i] < 10 * k && fonts[i - 1] + difference > fonts[i] && first_val+difference >= fonts[i])
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

	void process_image()
	{
		Pix *img = pixRead("E:/bachelor_thesis/tabularOCR/test_images/img/5.jpg");
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
			pixSetRGBPixel(img, textline->x, textline->y, 0, 255, 0);
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
			std::vector<int> all_spaces = get_spaces(line.first);
			int i = 0;
			while (i < font_cat.size() && line.second > font_cat[i].font)
				i++;
			if (i == font_cat.size())
			{
				font_cat[i - 1].lines.push_back(line.first);
				font_cat[i - 1].spaces.insert(font_cat[i - 1].spaces.end(), all_spaces.begin(), all_spaces.end());
			}
			else if (i == 0)
			{
				font_cat[0].lines.push_back(line.first);
				font_cat[0].spaces.insert(font_cat[0].spaces.end(), all_spaces.begin(), all_spaces.end());
			}
			else if (font_cat[i].font - line.second > line.second - font_cat[i - 1].font)
			{
				font_cat[i - 1].lines.push_back(line.first);
				font_cat[i - 1].spaces.insert(font_cat[i - 1].spaces.end(), all_spaces.begin(), all_spaces.end());
			}
			else
			{
				font_cat[i].lines.push_back(line.first);
				font_cat[i].spaces.insert(font_cat[i].spaces.end(), all_spaces.begin(), all_spaces.end());
			}	
		}

		// determine whitespace for each category
		for (int i = 0; i < font_cat.size(); i++)
		{
			int ws = img->w;
			double constant = font_cat[i].font / REF_FONT_SIZE;
			font_cat[i].whitespace = img->w;
			if (font_cat[i].spaces.size() != 0)
				font_cat[i].whitespace = get_whitespace(font_cat[i].spaces, constant);
		}

		for (size_t i = 0; i < all_lines.size(); i++)
		{
			// merge into words and add them into the all_words vector
			std::vector<BOX*> curr_words = merge_into_words(all_lines[i], line_map.at(all_lines[i]));
			for (size_t k = 0; k < curr_words.size(); k++)
			{
				set_border(img, curr_words[k], 255, 0, 0);
			}



			// determine whether the current line has columns or not
			/*
			std::vector<int> word_gaps = get_spaces(curr_words);
			std::vector<BOX*> columns = {};
			BOX* column = curr_words[0];
			for (size_t j = 0; j < word_gaps.size(); j++)
			{
				if (word_gaps[j] >= 3 * whitespace)
				{
					columns.push_back(column);
					set_border(img, column, 0, 255, 0);
					column = curr_words[j + 1];
				}
				else
					merge_boxes(column, curr_words[j + 1]);
			}
			if (columns.size() > 0)
			{
				set_border(img, column, 0, 255, 0);
				columns.push_back(column);
			}*/
			std::cout << "ALL:" << all_lines[i].size() << ":" << get_char_height(one_line) << std::endl;
		}

		std::string out = "E:/bachelor_thesis/tabularOCR/out5.jpg";
		char* path = &out[0u];
		pixWrite(path, img, IFF_PNG);
		api->End();
		pixDestroy(&img);


		// Get OCR result
		//char* outText = api->GetUTF8Text();
		//delete[] outText;
	}

}