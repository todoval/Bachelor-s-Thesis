#include "preprocess.h"

	void tabular_ocr::preprocessing::handle_preprocessing_error()
	{
		std::cerr << "Fatal error: wrong preprocessing parameters";
		exit(1);
	}

	tabular_ocr::preprocessing::preprocessor::preprocessor(image img_p, config cfg_p)
	{
		cfg = cfg_p;
		img = std::move(img_p);
	}

	void tabular_ocr::preprocessing::preprocessor::preprocess_file()
	{
		enhance();
		if (cfg.bin_method != binarization_method::NONE_B && cfg.gs_method == greyscale_method::NONE_G)
			cfg.gs_method = greyscale_method::LUMA;
		convert_to_greyscale();
		binarize();
		deskew();
	}

	void tabular_ocr::preprocessing::preprocessor::binarize()
	{
		Pix* binarized;
		if (cfg.bin_method == binarization_method::OTSU)
			pixOtsuAdaptiveThreshold(img.get(), 100, 100, 0, 0, 0.0f, NULL, &binarized);
		else if (cfg.bin_method == binarization_method::SAUVOLA)
			pixSauvolaBinarize(img.get(), 16, 0.5f, 1, NULL, NULL, NULL, &binarized);
		else if (cfg.bin_method == binarization_method::NONE_B)
			return;
		else
			handle_preprocessing_error();
		img = image(binarized);
	}

	void tabular_ocr::preprocessing::preprocessor::enhance()
	{
		if (cfg.en_method == enhancement_method::HIST_EQUALIZATION)
			pixEqualizeTRC(img.get(), img.get(), 0.5f, 2);
		else if (cfg.en_method == enhancement_method::SIMPLE)
			pixContrastTRC(img.get(), img.get(), 1);
		else if (cfg.en_method == enhancement_method::GAMMA)
			pixGammaTRC(img.get(), img.get(), 0.5f, 0, 255);
		else if (cfg.en_method == enhancement_method::NONE_E)
			return;
		else
			handle_preprocessing_error();
	}

	void tabular_ocr::preprocessing::preprocessor::convert_to_greyscale()
	{
		if (cfg.gs_method == greyscale_method::AVG)
		{
			Pix* gs = pixConvertRGBToGray(img.get(), 1/3.f, 1/3.f, 1/3.f);
			if (gs != nullptr && gs != img.get())
				img = image(gs);
		}
		else if (cfg.gs_method == greyscale_method::LUMA)
		{
			Pix* gs = pixConvertRGBToGray(img.get(), 0.2126f, 0.7152f, 0.0722f);
			if (gs != nullptr && gs != img.get())
				img = image(gs);
		}
		else if (cfg.gs_method == greyscale_method::MIN)
		{
			Pix* gs = pixConvertRGBToGrayMinMax(img.get(), L_CHOOSE_MIN);
			if (gs != nullptr && gs != img.get())
				img = image(gs);
		}
		else if (cfg.gs_method == greyscale_method::MAX)
		{
			Pix* gs = pixConvertRGBToGrayMinMax(img.get(), L_CHOOSE_MAX);
			if (gs != nullptr && gs != img.get())
				img = image(gs);
		}
		else if (cfg.gs_method == greyscale_method::NONE_G)
			return;
		else
			handle_preprocessing_error();
	}

	void tabular_ocr::preprocessing::preprocessor::deskew()
	{
		if (cfg.deskew)
		{
			Pix* deskewed = pixDeskew(img.get(), 0);
			if (deskewed != nullptr && deskewed != img.get())
				img = image(deskewed);
		}
	}

