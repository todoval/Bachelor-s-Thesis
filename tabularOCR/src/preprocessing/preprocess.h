#pragma once

#include "parser.h"

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

		// is thrown solely when an error in the implementation is encountered
		void handle_preprocessing_error();
	}
}