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

	void process_image(std::pair<std::string, cv::Mat> & image)
	{
		/*std::vector<std::string> test;
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/9.jpg");
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/2.jpg");
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/3.jpg");
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/73.jpg");
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/13.jpg");
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/8.jpg");
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/5.jpg");
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/table.jpg");
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/11.jpg");
		test.push_back("E:/bachelor_thesis/tabularOCR/test_images/img/7.jpg");
		std::vector<std::string> name;
		name.push_back("9");
		name.push_back("2");
		name.push_back("3");
		name.push_back("73");
		name.push_back("13");
		name.push_back("8");
		name.push_back("5");
		name.push_back("table");
		name.push_back("11");
		name.push_back("7");
		*/

		Pix *img = pixRead("E:/bachelor_thesis/tabularOCR/test_images/img/5.jpg");
		tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
		char *outText;
		if (api->Init(NULL, "eng", tesseract::OcrEngineMode::OEM_DEFAULT))
		{
			fprintf(stderr, "Could not initialize tesseract.\n");
			exit(1);
		}

		api->SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
		api->SetImage(img);
		api->SetVariable("textord_tabfind_find_tables", "true");
		api->SetVariable("textord_tablefind_recognize_tables", "true");
		api->Recognize(0);

		std::string out = "E:/bachelor_thesis/tabularOCR/5textline.jpg";
		char *path = &out[0u];

		Boxa * boxes = api->GetComponentImages(tesseract::RIL_TEXTLINE, false, NULL, NULL);
		for (size_t i = 0; i < boxes->n; i++)
		{
			BOX* box = boxaGetBox(boxes, i, L_CLONE);
			api->SetRectangle(box->x, box->y, box->w, box->h);
			set_border(img, box, 255, 0, 0);
			std::cout << i << ":" << box->x << ":" << box->y << ":" << box->w << ":" << box->h << ":" << ":" << std::endl;
		}
		pixWrite(path, img, IFF_PNG);

		img = pixRead("E:/bachelor_thesis/tabularOCR/test_images/img/5.jpg");
		api->SetImage(img);
		api->Recognize(0);
		Boxa * box2 = api->GetComponentImages(tesseract::RIL_WORD, false, NULL, NULL);
		for (size_t i = 0; i < box2->n; i++)
		{
			BOX* box = boxaGetBox(box2, i, L_CLONE);
			api->SetRectangle(box->x, box->y, box->w, box->h);
			set_border(img, box, 0, 255, 0);
			std::cout << i << ":" << box->x << ":" << box->y << ":" << box->w << ":" << box->h << ":" << ":" << std::endl;
		}
		out = "E:/bachelor_thesis/tabularOCR/5word.jpg";
		path = &out[0u];
		pixWrite(path, img, IFF_PNG);

		img = pixRead("E:/bachelor_thesis/tabularOCR/test_images/img/5.jpg");
		api->SetImage(img);
		api->Recognize(0);
		Boxa * box3 = api->GetComponentImages(tesseract::RIL_SYMBOL, false, NULL, NULL);
		for (size_t i = 0; i < box3->n; i++)
		{
			BOX* box = boxaGetBox(box3, i, L_CLONE);
			api->SetRectangle(box->x, box->y, box->w, box->h);
			//char* ocrResult = api->GetUTF8Text();
			//int conf = api->MeanTextConf();

			set_border(img, box, 0, 0, 255);
			std::cout << i << ":" << box->x << ":" << box->y << ":" << box->w << ":" << box->h << ":" << ":" << std::endl;
		}
		out = "E:/bachelor_thesis/tabularOCR/5symbol.jpg";
		path = &out[0u];
		pixWrite(path, img, IFF_PNG);

		img = pixRead("E:/bachelor_thesis/tabularOCR/test_images/img/5.jpg");
		api->SetImage(img);
		api->Recognize(0);
		Boxa * box4 = api->GetComponentImages(tesseract::RIL_BLOCK, false, NULL, NULL);
		for (size_t i = 0; i < box4->n; i++)
		{
			BOX* box = boxaGetBox(box4, i, L_CLONE);
			api->SetRectangle(box->x, box->y, box->w, box->h);
			//char* ocrResult = api->GetUTF8Text();
			set_border(img, box, 0, 0, 255);
			std::cout << i << ":" << box->x << ":" << box->y << ":" << box->w << ":" << box->h << ":" << ":" << std::endl;
		}
		out = "E:/bachelor_thesis/tabularOCR/5block.jpg";
		path = &out[0u];
		pixWrite(path, img, IFF_PNG);

		img = pixRead("E:/bachelor_thesis/tabularOCR/test_images/img/5.jpg");
		api->SetImage(img);
		api->Recognize(0);
		Boxa * box5 = api->GetComponentImages(tesseract::RIL_PARA, false, NULL, NULL);
		for (size_t i = 0; i < box5->n; i++)
		{
			BOX* box = boxaGetBox(box5, i, L_CLONE);
			api->SetRectangle(box->x, box->y, box->w, box->h);
			//char* ocrResult = api->GetUTF8Text();
			set_border(img, box, 255, 0, 255);
			std::cout << i << ":" << box->x << ":" << box->y << ":" << box->w << ":" << box->h << ":" << ":" << std::endl;
		}
		out = "E:/bachelor_thesis/tabularOCR/5para.jpg";
		path = &out[0u];
		pixWrite(path, img, IFF_PNG);


		tesseract::ResultIterator* ri = api->GetIterator();
		tesseract::PageIteratorLevel level = tesseract::RIL_BLOCK;
		if (ri == 0)
			return;

		do {
			const char* word = ri->GetUTF8Text(level);
			float conf = ri->Confidence(level);
			int x1, y1, x2, y2;
			ri->BoundingBox(level, &x1, &y1, &x2, &y2);
			auto type = ri->BlockType();
			int r, g, b;
			switch (type)
			{
			case PT_UNKNOWN: r = 0; g = 0; b = 0; break;
			case PT_FLOWING_TEXT: r = 255; g = 0; b = 0; break;
			case PT_HEADING_TEXT: r = 255; g = 0; b = 0; break;
			case PT_PULLOUT_TEXT: r = 255; g = 0; b = 0; break;
			case PT_EQUATION: r = 255; g = 255; b = 0; break;
			case PT_INLINE_EQUATION: r = 255; g = 255; b = 0; break;
			case PT_TABLE: r = 90; g = 60; b = 90; break;
			case PT_VERTICAL_TEXT: r = 255; g = 0; b = 0; break;
			case PT_CAPTION_TEXT: r = 255; g = 0; b = 0; break;
			case PT_FLOWING_IMAGE: r = 0; g = 0; b = 255; break;
			case PT_HEADING_IMAGE: r = 0; g = 0; b = 255; break;
			case PT_PULLOUT_IMAGE: r = 0; g = 0; b = 255; break;
			case PT_HORZ_LINE: r = 0; g = 255; b = 0; break;
			case PT_VERT_LINE: r = 0; g = 255; b = 0; break;
			case PT_NOISE: r = 0; g = 255; b = 255; break;
			case PT_COUNT: r = 0; g = 0; b = 0; break;
			}
			BOX box = { x1, y1, x2 - x1, y2 - y1 };
			set_border(img, &box, r, g, b);

			delete[] word;
		} while (ri->Next(level));
		//std::string output_path = "E:/bachelor_thesis/tabularOCR/vystup.jpg";
		//char *path = &output_path[0u];

		out = "E:/bachelor_thesis/tabularOCR/5vystup.jpg";
		path = &out[0u];
		pixWrite(path, img, IFF_PNG);

		//pixWrite(path, img, IFF_PNG);

		// Get OCR result
		outText = api->GetUTF8Text();
		printf("OCR output:\n%s", outText);

		// Destroy used object and release memory
		api->End();
		delete[] outText;
		pixDestroy(&img);
	}

}

/*
	
	



	
	*/
