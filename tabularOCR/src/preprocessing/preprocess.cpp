#include "preprocess.h"

namespace preprocessing
{

	void handle_preprocessing_error()
	{
		std::cout << "Wrong preprocessing parameters";
	}

	void preprocess_files(std::vector<file_info>& files, config cfg)
	{
		for (auto file : files)
		{
			Pix* to_process = file.preprocessed;
			binarize(cfg.bin_method, to_process);

		}
	}

	void binarize(binarization_method method, Pix * img)
	{
		if (method == binarization_method::OTSU)
			pixOtsuAdaptiveThreshold(img, 3, 3, 0, 0, 0.1, NULL, NULL);
		else if (method == binarization_method::SAUVOLA)
			pixSauvolaBinarize(img, 10, 0, 0, NULL, NULL, NULL, NULL);
		else if (method == binarization_method::NONE_B)
			return;
		else
			handle_preprocessing_error();
	}

	void enhance(enhancement_method method, Pix * img)
	{
		if (method == enhancement_method::HIST_EQUALIZATION)
			pixEqualizeTRC(img, img, 1, 3);
		else if (method == enhancement_method::SIMPLE)
			pixContrastTRC(img, img, 1);
		else if (method == enhancement_method::GAMMA)
			pixGammaTRC(img, img, 1, 0, 255);
		else if (method == enhancement_method::NONE_E)
			return;
		else
			handle_preprocessing_error();
	}

}