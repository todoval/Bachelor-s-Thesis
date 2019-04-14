#pragma once

#include "parser.h"


#ifdef _WIN32

#include "allheaders.h"

#else

#include "leptonica/allheaders.h"

#endif // _WIN32

namespace tabular_ocr
{
	namespace preprocessing
	{
		void handle_preprocessing_error();

		void preprocess_file(file_info & file, config cfg);

		void binarize(binarization_method method, Pix* img);

		void enhance(enhancement_method method, Pix* img);

		Pix* convert_to_greyscale(greyscale_method method, Pix ** img);

		Pix* deskew(Pix * img);

		// TO DO
		// noise reduction
		// scaling

	}
}