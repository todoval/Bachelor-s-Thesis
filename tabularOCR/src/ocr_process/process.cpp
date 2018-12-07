

#include "process.h"

namespace ocr
{

Process_info::Process_info()
{
}

}

/*
	//tesseract.setDatapath(System.getenv("TESSDATA_PREFIX"));
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	char *outText;
	// Initialize tesseract-ocr with English, without specifying tessdata path
	if (api->Init(NULL, "eng", tesseract::OcrEngineMode::OEM_DEFAULT)) {
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}
	// Open input image with leptonica library
	Pix *image = pixRead("test_images/img/5.jpg");
	//tesseract_preprocess(image);

	api->SetImage(image);
	api->Recognize(0);
	tesseract::ResultIterator* ri = api->GetIterator();
	tesseract::PageIteratorLevel level = tesseract::RIL_TEXTLINE;
	if (ri == 0)
		return 0;

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
		BOX box = { x1, y1, x2 - x1, y2 - y1 };
		//set_border(image, &box, r, g, b);

		delete[] word;
	} while (ri->Next(level));


	 pixWrite("test_images/temp/2.png", image, IFF_PNG);


	// Get OCR result
	outText = api->GetUTF8Text();
	printf("OCR output:\n%s", outText);

	// Destroy used object and release memory
	api->End();
	delete[] outText;
	pixDestroy(&image);
	
	

void set_border(PIX* img, BOX *box, int r, int g, int b) {
	for (int i = 0; i < box->w; i++) {
		pixSetRGBPixel(img, box->x + i, box->y, r, g, b);
		pixSetRGBPixel(img, box->x + i, box->y + box->h, r, g, b);
	}
	for (int i = 0; i < box->h; i++) {
		pixSetRGBPixel(img, box->x, box->y + i, r, g, b);
		pixSetRGBPixel(img, box->x + box->w, box->y + i, r, g, b);
	}
}

	
	*/
