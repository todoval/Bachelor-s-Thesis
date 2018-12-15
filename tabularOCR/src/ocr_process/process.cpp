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
		//Pix *img = matToPix(&image.second);

		Pix *img = pixRead("E:/bachelor_thesis/tabularOCR/test_images/img/9.jpg");

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
			switch (type) {
			case PT_UNKNOWN: r = 0; g = 0; b = 0; break;
			case PT_FLOWING_TEXT: r = 255; g = 0; b = 0; break;
			case PT_HEADING_TEXT: r = 255; g = 0; b = 0; break;
			case PT_PULLOUT_TEXT: r = 255; g = 0; b = 0; break;
			case PT_EQUATION: r = 255; g = 255; b = 0; break;
			case PT_INLINE_EQUATION: r = 255; g = 255; b = 0; break;
			case PT_TABLE: r = 255; g = 0; b = 255; break;
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
			std::cout << type << std::endl;
			BOX box = { x1, y1, x2 - x1, y2 - y1 };
			set_border(img, &box, r, g, b);

			delete[] word;
		} while (ri->Next(level));
		std::string output_path = "E:/bachelor_thesis/tabularOCR/vystup.jpg";
		char *path = &output_path[0u];
		pixWrite(path, img, IFF_PNG);

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
