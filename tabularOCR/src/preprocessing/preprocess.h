#pragma once

#include "parser.h"


#ifdef _WIN32

#include "allheaders.h"

#else

#include "leptonica/allheaders.h"

#endif // _WIN32


namespace preprocessing
{
	void handle_preprocessing_error();

	void preprocess_files(std::vector<file_info> & files, config cfg);

	void binarize(binarization_method method, Pix* img);

	void enhance(enhancement_method method, Pix* img);

	// noise reduction
	// deskew
	// contrast enhancement
	// scaling


}