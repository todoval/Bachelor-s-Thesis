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

	bool add_box(Box & result, Box & to_add)
	{
		if (result.w == 0)
		{
			result = to_add;
			return true;
		}
		if ( result.x >= to_add.x || abs(result.y - to_add.y) > 10
			|| (to_add.x > result.x + result.w + THRESHOLD))
			return false;
		result.w = to_add.w + to_add.x - result.x ;
		result.h = std::max(result.h, to_add.h); 
		return true;
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
		for (int i = 0; i < symbols.size() - 1; i++)
		{
			BOX* first = symbols[i];
			BOX* second = symbols[i + 1];
			int space = second->x - first->x - first->w;
			all_spaces.push_back(space);
		}
		std::sort(all_spaces.begin(), all_spaces.end());
		int max_ws = all_spaces[all_spaces.size() - 1] / 2;
		size_t i = all_spaces.size() - 1;
		while (all_spaces[i] > max_ws)
			i--;
		if (i < all_spaces.size() - 1)
			i++;
		for (int i = 0; i < all_spaces.size(); i++)
			std::cout << all_spaces[i] << ":";
		std::cout << "RESULT:" << all_spaces[i] << std::endl;
		return all_spaces[i];
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
			}
			else
			{
				result.push_back(word);
				word = symbols[i + 1];
			}
		}
 		return result;
	}

	void process_image(std::pair<std::string, cv::Mat> & image)
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
			int whitespace = 0;
			if (one_line.size() > 1)
				whitespace = get_whitespace(one_line);
			// merge into words and add them into the all_words vector
 			std::vector<BOX*> curr_words = merge_into_words(one_line, whitespace);
			for (size_t k = 0; k < curr_words.size(); k++)
			{
				set_border(img, curr_words[k], 255, 0, 0);
			}
			all_words.insert(all_words.end(),  curr_words.begin(), curr_words.end());
			std::cout << one_line.size() << ":" << whitespace << std::endl;
			one_line = {};
		}

		std::string out = "E:/bachelor_thesis/tabularOCR/out5.jpg";
		char* path = &out[0u];
		pixWrite(path, img, IFF_PNG);
		api->End();
		pixDestroy(&img);


		/**
		BOX curr_box = { 0,0,0,0,0 };
		for (size_t i = 0; i < comp_arr->n; i++)
		{
			
			BOX* new_box = boxaGetBox(comp_arr, i, L_CLONE);
			if (!add_box(curr_box, *new_box))
			{
				set_border(img, &curr_box, 255, 0, 255);
				std::cout << curr_box.x << ":" << curr_box.y << std::endl;
				curr_box = { 0,0,0,0,0 };
				continue;
			}
			api->SetRectangle(new_box->x, new_box->y, new_box->w, new_box->h);
			//char* ocrResult = api->GetUTF8Text();
			set_border(img, new_box, 0, 0, 255);
			std::cout << i << ":" << new_box->x << ":" << new_box->y << ":" << ":" << std::endl;
		}

		do {
			const char* symbol = ri->GetUTF8Text(level);
			float conf = ri->Confidence(level);
			if (conf < 90)
				continue;
			int left, top, right, bottom;
			ri->BoundingBox(level, &left, &top, &right, &bottom);
			BOX new_box = { left, top, right - left, bottom - top };

			if (!add_box(curr_box, new_box))
			{
				set_border(img, &curr_box, 255, 0, 255);
				std::cout << curr_word << std::endl;
				curr_box = { 0,0,0,0,0 };
				curr_word = "";
				continue;
			}
			curr_word += symbol;
			//auto type = ri->BlockType();
			//BOX box = { x1, y1, x2 - x1, y2 - y1 };
			//set_border(img, &box, 255, 0, 255);
			delete[] symbol;
		}
		while (ri->Next(level));
		*/
		// Get OCR result
		//char* outText = api->GetUTF8Text();
		//printf("OCR output:\n%s", outText);

		// Destroy used object and release memory

		//delete[] outText;
	}

}