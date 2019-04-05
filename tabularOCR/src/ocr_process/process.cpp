#include "process.h"

namespace ocr
{
	page::page()
	{
		img = pixRead("D:/bachelor_thesis/tabularOCR/test_images/img/5-1.jpg");
		page(img);
	}

	page::page(const std::string & file_name)
	{
		filename = file_name;
		img = pixRead(file_name.c_str());
		// itialize tesseract api without the use of LSTM
		api = new tesseract::TessBaseAPI();
		if (img->d == 8)
			img = pixConvert8To32(img);
		init_api(img);
	}

	void page::set_border(std::unique_ptr<BOX> & box, int r, int g, int b)
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

	bool page::is_symbol_in_textline(std::unique_ptr<BOX> & symbol, std::unique_ptr<BOX> & textline)
	{
		return (symbol->x <= textline->x + textline->w
			&& symbol->x >= textline->x
			&& symbol->y >= textline->y
			&& symbol->y <= textline->y + textline->h);
	}

	std::vector<int> page::get_spaces(const std::vector<std::unique_ptr<BOX>> & symbols)
	{
		std::vector <int> result;
		// iterate over all symbols and return the whitespaces between them
		auto size = static_cast<int> (symbols.size() - 1);
		for (int i = 0; i < size; i++)
		{
			auto first = &symbols[i];
			auto second = &symbols[i + 1];
			int space = second->get()->x - first->get()->x - first->get()->w;
			result.push_back(space);
		}
		return result;
	}

	int page::get_whitespace(std::vector<int> & all_spaces, double constant)
	{
		std::sort(all_spaces.begin(), all_spaces.end());
		// heuristical estimation of the space

		// find greatest element that is smaller than constant
		auto it = all_spaces.begin();
		for (it = all_spaces.begin(); it != all_spaces.end(); it++)
		{
			if (*it > constant)
				break;
		}

		if (it == all_spaces.end())
		{
			if (all_spaces.empty())
				return 0;
			return all_spaces.back();
		}

		for (it; it != all_spaces.end() - 1; it++)
		{
			double multi_factor = get_multi_factor(*it, constant);
			auto k = *std::next(it);
			if (*std::next(it) >= multi_factor * *it)
				break;
		}
		if (it != all_spaces.end())
			return *it;
		return 0;
	}

	line page::merge_lines(line & first, line & second, std::map<int, int>& no_of_cols)
	{
		line result;

		// get resulting heights and y-axis of the resulting line

		int first_h = get_char_height(first, img->w);
		int sec_h = get_char_height(second, img->w);
		int first_y = get_y_axis(first);
		int sec_y = get_y_axis(second);

		// calculate the y axis and height of all the boxes

		int y = first_y;
		int height = sec_h + sec_y - first_y;

		// used to keep track of already merged columns from second line
		std::vector<int> second_line_indices;
		for (int i = 0; i < second.size(); i++)
			second_line_indices.push_back(i);

		// go over the first vector and either add from map or at only this box into the new vector
		for (size_t i = 0; i < first.size(); i++)
		{
			auto bbox = boxCopy(first[i].get());
			if (bbox == nullptr)
				return {};
			auto new_col = std::unique_ptr<BOX>(bbox);
			new_col->h = height;
			new_col->y = y;
			// if these columns overlap, add them to the resulting vector
			if (no_of_cols.find(i) != no_of_cols.end())
			{
				int sec_index = no_of_cols[i];
				second_line_indices.erase(std::find(second_line_indices.begin(), second_line_indices.end(), sec_index));

				new_col->x = std::min(first[i]->x, second[sec_index]->x);
				new_col->w = get_width_of_col(first[i], second[sec_index]);
			}
			// if they don't overlap, try to find a suitable column from the second line to merge it with
			else
			{
				bool is_set = false;
				for (auto j : second_line_indices)
				{
					if (overlap(first[i], second[j]))
					{
						// merge these two together
						is_set = true;
						new_col->x = std::min(first[i]->x, second[j]->x);
						new_col->w = get_width_of_col(first[i], second[j]);
						second_line_indices.erase(std::find(second_line_indices.begin(), second_line_indices.end(), j));
						break;
					}
				}
				if (!is_set)
				{
					new_col->x = first[i]->x;
					new_col->w = first[i]->w;
				}
			}
			result.push_back(std::move(new_col));
		}
		// push back second line columns that weren't merged
		for (auto i : second_line_indices)
		{
			auto new_col = std::unique_ptr<BOX>(boxCopy(second[i].get()));
			result.push_back(std::move(new_col));
		}
		std::sort(result.begin(), result.end(), [](auto & a, auto & b) {return a->x < b->x; });
		for (size_t i = 0; i < result.size(); i++)
		{
		//	set_border(result[i], 255,0,0);
		}
		return std::move(result);
	}

	void page::box_merge_vertical(std::unique_ptr<BOX> & result, std::unique_ptr<BOX> & to_add)
	{
		if (result == nullptr)
			result = std::move(to_add);
		else
		{
			result->w = std::max(result->w, to_add->w);
			result->x = std::min(result->x, to_add->x);
			result->y = std::min(result->y, to_add->y);
			result->h = to_add->h + to_add->y - result->y;
		}
	}

	void page::box_merge_horizontal(std::unique_ptr<BOX> & result, std::unique_ptr<BOX> &to_add)
	{
		if (result == nullptr)
			result = std::move(to_add);
		else
		{
			result->w = to_add->w + to_add->x - result->x;
			result->h = std::max(result->h, to_add->h);
			result->y = std::min(result->y, to_add->y);
		}
	}

	std::vector<std::unique_ptr<BOX>> page::merge_into_words(std::vector<std::unique_ptr<BOX>> & symbols, int whitespace)
	{
		std::sort(symbols.begin(), symbols.end(),
			[](auto & a, auto & b) { return a->x < b->x; });

		std::vector<std::unique_ptr<BOX>> result = {};
		auto word = std::unique_ptr<BOX>(boxCopy(symbols[0].get()));
		for (size_t i = 0; i < symbols.size() - 1; i++)
		{
			if (symbols[i]->x + symbols[i]->w + whitespace >= symbols[i + 1]->x)
				box_merge_horizontal(word, symbols[i + 1]);
			else
			{
				result.push_back(std::move(word));
				word = std::unique_ptr<BOX>(boxCopy(symbols[i + 1].get()));
			}
		}
		result.push_back(std::move(word));
		return std::move(result);
	}

	std::vector<std::unique_ptr<BOX>> page::merge_into_columns(std::vector<std::unique_ptr<BOX>> & words, int whitespace)
	{
		std::vector<int> word_gaps = get_spaces(words);
		std::vector<std::unique_ptr<BOX>> columns = {};
		auto column = std::unique_ptr<BOX>(boxCopy(words[0].get()));

		for (size_t j = 0; j < word_gaps.size(); j++)
		{
			if (word_gaps[j] >= 4.1 * whitespace)
			{
				columns.push_back(std::move(column));
				column = std::unique_ptr<BOX>(boxCopy(words[j+1].get()));
			}
			else
				box_merge_horizontal(column, words[j + 1]);
		}
		columns.push_back(std::move(column));
		return std::move(columns);
	}

	void page::process_cat_and_init(int & cat_font, std::vector<std::shared_ptr<textline>> & cat_lines, int & first_val,
		std::multimap<int, std::shared_ptr<textline> >::iterator it)
	{
		process_category(cat_font, cat_lines, first_val);

		// next category initialization

		first_val = it->first;
		cat_font = first_val;
		cat_lines = { it->second };
	}

	void page::process_category(int & cat_font, std::vector<std::shared_ptr<textline>> & cat_lines, int & first_val)
	{
		// calculate the font for current category
		cat_font /= cat_lines.size();

		//determine whitespace

		double constant = cat_font / REF_FONT_SIZE;
		std::vector<int> cat_spaces;
		for (auto line : cat_lines)
		{
			auto spaces = get_spaces(line->symbols);
			int whitespace = get_whitespace(spaces, constant);
			if (whitespace != img->w && whitespace != 0)
				cat_spaces.push_back(whitespace);

		}
		int cat_ws = most_common_number(cat_spaces); // category whitespace
		for (auto line : cat_lines)
		{
			if (line->symbols.size() <= 1)
				line->whitespace = 0;
			else
			{
				line->whitespace = cat_ws;
				auto words = merge_into_words(line->symbols, line->whitespace);
				//	for (auto e : words)
					//	set_border(e, 255, 0, 255);
				line->columns = merge_into_columns(words, line->whitespace);
				//for (auto e : line->columns)
				//	set_border(e, 255, 0, 255);
			}
		}
	}

	void page::determine_columns(std::multimap<int, std::shared_ptr<textline>> & fonts)
	{
		// constants that will be used to determine whitespaces
		int difference = 4;
		int k = 1;

		// iterate over all the lines and create font categories - categories of lines with the same font
		int first_val = fonts.begin()->first;

		// initialize first category
		int cat_font = first_val; // the font if the current category
		std::vector<std::shared_ptr<textline>> cat_lines = { fonts.begin()->second }; // all the lines that are in the current category
		auto it = std::next(fonts.begin());
		for (it; it != fonts.end(); ++it)
		{
			while (it->first >= 10 * k)
			{
				k++;
				difference++;
			}
			// add an element to the current category
			if (first_val + difference >= it->first)
			{
				cat_font += it->first;
				cat_lines.push_back(it->second);
			}
			else // process current category
			{
				process_cat_and_init(cat_font, cat_lines, first_val, it);
			}
		}
		process_category(cat_font, cat_lines, first_val);
	}

	void page::create_table(table & curr_table, std::vector<std::unique_ptr<BOX>> & merged_cols)
	{
		if (!curr_table.textlines.empty())
		{
			// TO DO - create a real table
			curr_table.rows = curr_table.textlines.size();
			curr_table.cols = merged_cols.size();
			curr_table.table_repres = merge_to_table(merged_cols);
			curr_table.column_repres = std::move(merged_cols);

			all_tables.push_back(std::move(curr_table));

			merged_cols.clear();
		}
	}

	std::vector<table> page::merge_cols(std::vector<std::shared_ptr<textline>> & page)
	{
		// the resulting vector of all tables in adequate structure
		// vector of textlines that represent one table merged only by columns
		std::vector<std::unique_ptr<BOX>> merged_cols;
		table curr_table;
		for (size_t i = 0; i < page.size() - 1; i++)
		{
			// check two lines that are under each other whether they are in the same table
			auto first = &page[i]->columns;
			if (!merged_cols.empty())
				first = &merged_cols;
			auto second = &page[i + 1]->columns;
			// map for saving all the indexes of columns for further merging
			std::map<int, int> no_of_cols;
			// iterators used for iterating over first and second line
			size_t iter_one = 0;
			size_t iter_two = 0;

			if (second->size() == 4)
			{
				for (int k = 0; k < 4; k++);
				//	set_border((*second)[k], 0, 255, 0);
			}

			// cycle that iterates over columns of the two lines
			while (true)
			{
				// end of cycle
				if (iter_one >= first->size() || iter_two >= second->size())
				{
					// if at least one pair is found, merge
					if (no_of_cols.size() > 0)
					{
						if (curr_table.textlines.empty())
							curr_table.textlines.push_back(page[i]);
						curr_table.textlines.push_back(page[i + 1]);
						if (merged_cols.empty())
							merged_cols = merge_lines(page[i]->columns, page[i + 1]->columns, no_of_cols);
						else merged_cols = merge_lines(merged_cols, page[i + 1]->columns, no_of_cols);
					}
					// if there was no match but a table already exists
					else
						create_table(curr_table, merged_cols);
					break;
				}

				// get to the same column in both lines
				if (is_most_left(first->at(iter_one), second->at(iter_two))
					&& !are_in_same_col(first->at(iter_one), second->at(iter_two)))
				{
					iter_one++;
					continue;
				}
				if (is_most_left(second->at(iter_two), first->at(iter_one))
					&& !are_in_same_col(first->at(iter_one), second->at(iter_two)))
				{
					iter_two++;
					continue;
				}

				// both are in the same column

				// check whether the chosen columns don't overlap with anything else
				bool to_merge = true;
				for (size_t k = 0; k < first->size(); k++)
				{
					if (k == iter_one)
						continue;
					if (overlap(first->at(k), second->at(iter_two)))
						to_merge = false;
				}
				for (size_t k = 0; k < second->size(); k++)
				{
					if (k == iter_two)
						continue;
					if (overlap(second->at(k), first->at(iter_one)))
						to_merge = false;
				}

				// if they don't, merge these two columns
				if (to_merge)
					no_of_cols.insert(std::pair<int, int>(iter_one, iter_two));
				else
				{
					// not going to merge but table may already exist
					create_table(curr_table, merged_cols);
					no_of_cols.clear();
					break;
				}
				iter_two++;
				iter_one++;
			}
		}
		create_table(curr_table, merged_cols);
		return std::move(all_tables);
	}

	table::table()
	{
	}

	void page::delete_footer()
	{
		// TO-DO - calculate footer threshold by something different than a constant

		// the last element in the textline vector will be the one that is in the footer

		// deal with multi-line footers
		std::vector<std::shared_ptr<textline>>::reverse_iterator it = textlines.rbegin();
		for (it = textlines.rbegin(); it != textlines.rend() - 1 ; it++)
		{
			int line_diff = get_y_axis(it->get()->columns) - get_y_axis(std::next(it)->get()->columns) - get_char_height(std::next(it)->get()->columns, img->w);
			if (line_diff > FOOTER_THRESHOLD)
				break;
		}
		auto forward_it = (it + 1).base();
		textlines.erase(forward_it, textlines.end());
	}

	void page::init_api(Pix *img)
	{
		if (api->Init(NULL, "eng", tesseract::OcrEngineMode::OEM_TESSERACT_ONLY))
		{
			fprintf(stderr, "Could not initialize tesseract.\n");
			exit(1);
		}

		api->SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
		api->SetImage(img);
		api->SetVariable("textord_tabfind_find_tables", "true");
		api->SetVariable("textord_tablefind_recognize_tables", "true");
		api->Recognize(0);
	}

	textline::textline()
	{
	}

	std::multimap<int, std::shared_ptr<textline>> page::init_textlines()
	{
		// array of symbols that exist in the image
		Boxa * symbol_arr = api->GetComponentImages(tesseract::RIL_SYMBOL, false, NULL, NULL);
		// array of textlines that exist in the image
		Boxa * textline_arr = api->GetComponentImages(tesseract::RIL_TEXTLINE, false, NULL, NULL);

		// sort textline array 
		std::sort(textline_arr->box, textline_arr->box,
			[](BOX* & a, BOX* & b) { return a->y < b->y; });

		std::unique_ptr<BOX> curr_line;
		std::unique_ptr<BOX> curr_symbol;
		std::shared_ptr<textline> line = std::make_shared<textline>();
		textlines = std::vector<std::shared_ptr<textline>>(textline_arr->n);

		std::multimap<int, std::shared_ptr<textline>> fonts;

		// iterate over textlines and symbols and initialize all textlines with symbols that belong to them

		for (size_t i = 0; i < textline_arr->n; i++)
		{
			curr_line = std::unique_ptr<BOX>(boxCopy(textline_arr->box[i]));
			for (size_t j = 0; j < symbol_arr->n; j++)
			{
				curr_symbol = std::unique_ptr<BOX>(boxCopy(symbol_arr->box[j]));
				if (is_symbol_in_textline(curr_symbol, curr_line))
					line->symbols.push_back(std::move(curr_symbol));
			}
			int height = get_char_height(line->symbols, img->w);
			textlines[i] = line;
			textlines[i]->font = height;
			fonts.insert({ height, textlines[i] });
			line = std::make_shared<textline>();
		}
		delete[] symbol_arr;
		delete[] textline_arr;

		return fonts;
	}

	void page::delete_unusual_lines()
	{
		while (true)
		{
			auto it = std::find_if(textlines.begin(), textlines.end(), [this](auto line)
			{ return line->whitespace == 0 || line->symbols.empty() || is_textline_table(line);
			});
			if (it != textlines.end())
				textlines.erase(it);
			else
				break;
		}
	}

	std::unique_ptr<BOX> page::merge_to_table(std::vector<std::unique_ptr<BOX>> & cols)
	{
		if (cols.empty())
			return nullptr;
		auto result = std::unique_ptr<BOX>(boxCopy(cols[0].get()));
		if (result == nullptr)
			return nullptr;
		if (cols.size() > 1)
			result->w = cols[cols.size() - 1]->x - cols[0]->x + cols[cols.size() - 1]->w;
		return std::move(result);
	}

	bool page::is_textline_table(std::shared_ptr<textline> line)
	{
		for (auto & symbol : line->symbols)
		{
			if (symbol->h > line->font * 5)
				return true;
		}
		return false;
	}

	void page::process_image()
	{
		// initializes the textlines vector - adds symbols and current font to the textlines
		auto fonts = init_textlines();

		// save information about columns into the textline vector
		determine_columns(fonts);

		delete_unusual_lines();

		/*
		delete footer could probably be implemented before determination of font categories
		pros - simpler, less time consuming function, only one iteration
		cons - multiline footers
		*/
		delete_footer();

		// merge columns into tables

		all_tables = merge_cols(textlines);

		for (int i = 0; i < all_tables.size(); i++)
		{
			for (int j = 0; j < all_tables[i].column_repres.size(); j++)
			{
				if (all_tables[i].column_repres.size() > 1);
					set_border(all_tables[i].column_repres[j], 255, 0, 0);
				//	set_border(all_tables[i].table_repres, 0, 0, 255);
			}

		}

		std::string out = "results/" + get_filename(filename) + ".png";
		//std::string out = "results/5.png";

		char* path = &out[0u];
		pixWrite(path, img, IFF_PNG);
		api->End();
		pixDestroy(&img);

	}

}
/*
TO DO:
- determine footer threshold differently than by a constant
- single row tables -> delete
- preprocess multiple dots ... in an image -> delete
- allow minor mistakes in alingments
- full-page tables - whitespace problem, determine whitespace differently? 

*/