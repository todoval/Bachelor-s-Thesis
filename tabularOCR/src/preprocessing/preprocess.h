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

		class preprocessor
		{
		public:
			image img;
			config cfg;

			preprocessor(image img_p, config cfg_p);

			void preprocess_file();

		private:

			void binarize();

			void enhance();

			void convert_to_greyscale();

			void deskew();
		};

		void handle_preprocessing_error();
	}
}