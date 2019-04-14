#include "preprocess.h"

	void tabular_ocr::preprocessing::handle_preprocessing_error()
	{
		std::cout << "Wrong preprocessing parameters";
	}

	void tabular_ocr::preprocessing::preprocess_file(file_info & file, config cfg)
	{
		enhance(cfg.en_method, file.preprocessed);

		if (cfg.bin_method != binarization_method::NONE_B && cfg.gs_method == greyscale_method::NONE_G)
		{
			auto greyscale = convert_to_greyscale(greyscale_method::LUMA, &file.preprocessed);
			pixDestroy(&file.preprocessed);
			file.preprocessed = greyscale;
		}
		else
		{
			auto greyscale = convert_to_greyscale(cfg.gs_method, &file.preprocessed);
			if (greyscale != nullptr)
			{
				pixDestroy(&file.preprocessed);
				file.preprocessed = greyscale;
			}
		}

		binarize(cfg.bin_method, file.preprocessed);

		if (cfg.deskew)
		{
			auto deskewed = convert_to_greyscale(cfg.gs_method, &file.preprocessed);
			pixDestroy(&file.preprocessed);
			file.preprocessed = deskewed;
		}
	}

	void tabular_ocr::preprocessing::binarize(binarization_method method, Pix * img)
	{
		if (method == binarization_method::OTSU)
			pixOtsuAdaptiveThreshold(img, 0, 100, 100, 0, 0, NULL, &img);
		else if (method == binarization_method::SAUVOLA)
			pixSauvolaBinarize(img, 16, 0.5, 1, NULL, NULL, NULL, &img);
		else if (method == binarization_method::NONE_B)
			return;
		else
			handle_preprocessing_error();
	}

	void tabular_ocr::preprocessing::enhance(enhancement_method method, Pix * img)
	{
		if (method == enhancement_method::HIST_EQUALIZATION)
			pixEqualizeTRC(img, img, 0.5, 2);
		else if (method == enhancement_method::SIMPLE)
			pixContrastTRC(img, img, 1);
		else if (method == enhancement_method::GAMMA)
			pixGammaTRC(img, img, 0.5, 0, 255);
		else if (method == enhancement_method::NONE_E)
			return;
		else
			handle_preprocessing_error();
	}

	Pix* tabular_ocr::preprocessing::convert_to_greyscale(greyscale_method method, Pix ** img)
	{
		if (method == greyscale_method::AVG)
			return pixConvertRGBToGray(*img, 1 / 3, 1 / 3, 1 / 3);
		else if (method == greyscale_method::LUMA)
			return pixConvertRGBToGray(*img, 0.2126, 0.7152, 0.0722);
		else if (method == greyscale_method::MIN)
			return pixConvertRGBToGrayMinMax(*img, L_CHOOSE_MIN);
		else if (method == greyscale_method::MAX)
			return pixConvertRGBToGrayMinMax(*img, L_CHOOSE_MAX);
		else if (method == greyscale_method::NONE_G)
			return nullptr;
		else
			handle_parsing_error();
		return nullptr;
	}

	Pix* tabular_ocr::preprocessing::deskew(Pix * img)
	{
		return pixDeskew(img, 0);
	}


