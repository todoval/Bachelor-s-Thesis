#pragma once

#include "parser.h"
#include "opencv2/core.hpp"
#include "opencv2/photo/photo.hpp"


#ifdef _WIN32

#include "allheaders.h"

#else

#include "leptonica/allheaders.h"

#endif // _WIN32


namespace preprocessing
{
	// noise reduction
	// deskew
	// binarize
	// contrast enhancement
	// scaling

	preprocess();


}