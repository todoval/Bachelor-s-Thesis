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

	int get_whitespace(std::vector<BOX*> & symbols)
	{
		std::vector <int> all_spaces;
		// iterate over all symbols and return the whitespaces between them
		for (int i = 0; i < symbols.size() - 1; i++)
		{
			BOX* first = symbols[i];
			BOX* second = symbols[i + 1];
			int space = second->x - first->x - first->w;
			all_spaces.push_back(space);
		}
		std::sort(all_spaces.begin(), all_spaces.end());
		// heuristical estimation of the space
		double constant = get_avg_char_width(symbols) / REF_FONT_SIZE;
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

		/*
		int i = all_spaces.size() - 1;
		while (i > 0)
		{
			double comp = all_spaces[i] / 2;
			if (all_spaces[i] % 2 == 1)
				comp++;
			if (all_spaces[i - 1] <= comp)
				break;
			i--;
		}
		*/
		int ws = 0;
		if (i < all_spaces.size())
			ws = all_spaces[i];
		for (int i = 0; i < all_spaces.size(); i++)
			std::cout << all_spaces[i] << ":";
		std::cout << "RESULT:" << ws << std::endl;
		return ws;
	}

	std::vector<BOX*> merge_into_words(std::vector<BOX*> & symbols, int whitespace)
	{
		std::vector<BOX*> result = {};
		BOX* word = symbols[0]; 
		for (size_t i = 0; i < symbols.size() - 1; i++)
		{	
			if (symbols[i]->x + symbols[i]->w + whitespace > symbols[i + 1]->x)
			{
				word->w = symbols[i + 1]->w + symbols[i + 1]->x - word->x;
				word->h = std::max(word->h, symbols[i + 1]->h);
				word->y = std::min(word->y, symbols[i + 1]->y);
			}
			else
			{
				result.push_back(word);
				word = symbols[i + 1];
			}
		}
		result.push_back(word);
 		return result;
	}

	int get_avg_char_width(std::vector<BOX*> & symbols)
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

		std::vector<BOX*> one_line = {};
		std::vector <BOX*> all_words = {};
		/*
		iterate over textlines and symbols to find all symbols in current textline 
		once found, getwhitespace separating different words in each textline
		merge these words together and output save them to a vector
		*/
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
			// get the whitespace of the current line
			int whitespace = img->w;
			if (one_line.size() > 1)
				whitespace = get_whitespace(one_line);
			// if line size and whitespace size is too low, it means that all symbols belong to one word)
			if (one_line.size() < MAX_CHARS_IN_WORD && whitespace < one_line[0]->w)
				whitespace = img->w;
			// merge into words and add them into the all_words vector
 			std::vector<BOX*> curr_words = merge_into_words(one_line, whitespace);
			for (size_t k = 0; k < curr_words.size(); k++)
			{
				set_border(img, curr_words[k], 255, 0, 0);
			}
			all_words.insert(all_words.end(),  curr_words.begin(), curr_words.end());
			std::cout << "ALL:" << one_line.size() << ":" << whitespace << ":" << get_avg_char_width(one_line) << std::endl;
			one_line = {};
		}


		std::string out = "E:/bachelor_thesis/tabularOCR/out5.jpg";
		char* path = &out[0u];
		pixWrite(path, img, IFF_PNG);
		api->End();
		pixDestroy(&img);


		// Get OCR result
		//char* outText = api->GetUTF8Text();
		//printf("OCR output:\n%s", outText);

		// Destroy used object and release memory

		//delete[] outText;
	}

}