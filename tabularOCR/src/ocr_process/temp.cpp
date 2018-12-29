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

	}

	void process_image(std::pair<std::string, cv::Mat> & image)
	{
		Pix *img = pixRead("E:/bachelor_thesis/tabularOCR/test_images/img/3.jpg");
		if (img->d == 8)
			img = pixConvert8To32(img);
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

		tesseract::ResultIterator* ri = api->GetIterator();
		tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
		if (ri == 0)
			return;

		Boxa * symbol_arr = api->GetComponentImages(tesseract::RIL_SYMBOL, false, NULL, NULL);
		Boxa * textline_arr = api->GetComponentImages(tesseract::RIL_TEXTLINE, false, NULL, NULL);

		int textline_no = 0;
		BOX* textline = boxaGetBox(symbol_arr, textline_no, L_CLONE);
		std::vector<BOX*> one_line = {};
		for (size_t i = 0; i < symbol_arr->n; i++)
		{
			BOX* symbol = boxaGetBox(symbol_arr, i, L_CLONE);
			if (is_symbol_in_textline(symbol, textline))
			{
				one_line.push_back(symbol);
			}
			else
			{

			}
		}

		BOX curr_box = { 0,0,0,0,0 };
		for (size_t i = 0; i < comp_arr->n; i++)
		{
			
			BOX* new_box = boxaGetBox(comp_arr, i, L_CLONE);
			/*
			if (!add_box(curr_box, *new_box))
			{
				set_border(img, &curr_box, 255, 0, 255);
				std::cout << curr_box.x << ":" << curr_box.y << std::endl;
				curr_box = { 0,0,0,0,0 };
				continue;
			}*/
			api->SetRectangle(new_box->x, new_box->y, new_box->w, new_box->h);
			//char* ocrResult = api->GetUTF8Text();
			set_border(img, new_box, 0, 0, 255);
			std::cout << i << ":" << new_box->x << ":" << new_box->y << ":" << ":" << std::endl;
		}

		/*


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

		std::string out = "E:/bachelor_thesis/tabularOCR/out3.jpg";
		char* path = &out[0u];
		pixWrite(path, img, IFF_PNG);

		// Get OCR result
		//char* outText = api->GetUTF8Text();
		//printf("OCR output:\n%s", outText);

		// Destroy used object and release memory
		api->End();
		//delete[] outText;
		pixDestroy(&img);
	}

}