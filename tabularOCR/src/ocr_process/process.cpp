#include "process.h"

namespace ocr
{
	page::page()
	{
		img = pixRead("D:/bachelor_thesis/tabularOCR/test_images/img/5-1.jpg");
		page(img);
	}

	page::page(const std::string & filename)
	{
		img = pixRead(filename.c_str());
		// itialize tesseract api without the use of LSTM
		api = new tesseract::TessBaseAPI();
		if (img->d == 8)
			img = pixConvert8To32(img);
		init_api(img);
		textlines = {};
		all_tables = {};
	}

	void page::set_border(BOX *box, int r, int g, int b)
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

	bool page::is_symbol_in_textline(BOX* symbol, BOX* textline)
	{
		return (symbol->x <= textline->x + textline->w
			&& symbol->x >= textline->x
			&& symbol->y >= textline->y
			&& symbol->y <= textline->y + textline->h);
	}

	std::vector<int> page::get_spaces(const line & symbols)
	{
		std::vector <int> result;
		// iterate over all symbols and return the whitespaces between them
		auto size = static_cast<int> (symbols.size() - 1);
		for (int i = 0; i < size; i++)
		{
			BOX* first = symbols[i];
			BOX* second = symbols[i + 1];
			int space = second->x - first->x - first->w;
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

		for (it; it != all_spaces.end()-1; it++)
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

		// go over the first vector and either add from map or at only this box into the new vector
		for (size_t i = 0; i < first.size(); i++)
		{
			BOX* new_col = first[i];
			new_col->h = height;
			new_col->y = y;
			if (no_of_cols.find(i) != no_of_cols.end())
			{
				int sec_index = no_of_cols[i];
				new_col->x = std::min(first[i]->x, second[sec_index]->x);
				new_col->w = get_width_of_col(first[i], second[sec_index]);
			}
			else
			{
				new_col->x = first[i]->x;
				new_col->w = first[i]->w;
			}
			result.push_back(new_col);
		}
		return result;
	}

	void page::box_merge_vertical(BOX* result, BOX* to_add)
	{
		if (result == nullptr)
			result = to_add;
		else
		{
			result->w = std::max(result->w, to_add->w);
			result->x = std::min(result->x, to_add->x);
			result->y = std::min(result->y, to_add->y);
			result->h = to_add->h + to_add->y - result->y;
		}
	}

	void page::box_merge_horizontal(BOX* result, BOX* to_add)
	{
		if (result == nullptr)
			result = to_add;
		else
		{
			result->w = to_add->w + to_add->x - result->x;
			result->h = std::max(result->h, to_add->h);
			result->y = std::min(result->y, to_add->y);
		}
	}

	std::vector<BOX*> page::merge_into_words(std::vector<BOX*> & symbols, int whitespace)
	{
		std::sort(symbols.begin(), symbols.end(),
			[](BOX* & a, BOX* & b) { return a->x < b->x; });

		std::vector<BOX*> result = {};
		auto word = boxCopy(symbols[0]);
		for (size_t i = 0; i < symbols.size() - 1; i++)
		{
			if (symbols[i]->x + symbols[i]->w + whitespace >= symbols[i + 1]->x)
				box_merge_horizontal(word, symbols[i + 1]);
			else
			{
				result.push_back(word);
				word = boxCopy(symbols[i + 1]);
			}
		}
		result.push_back(word);
		return result;
	}

	std::vector<BOX*> page::merge_into_columns(std::vector<BOX*> & words, int whitespace)
	{
		std::vector<int> word_gaps = get_spaces(words);
		std::vector<BOX*> columns = {};
		BOX* column = words[0];
		for (size_t j = 0; j < word_gaps.size(); j++)
		{
			if (word_gaps[j] >= 4.1 * whitespace)
			{
				columns.push_back(column);
				column = words[j + 1];
			}
			else
				box_merge_horizontal(column, words[j + 1]);
		}
		columns.push_back(column);
		return columns;
	}

	void page::process_cat_and_init(int & cat_font, std::vector<textline *> & cat_lines, int & first_val,
		std::multimap<int, textline * >::iterator it)
	{
		process_category(cat_font, cat_lines, first_val);

		// next category initialization

		first_val = it->first;
		cat_font = first_val;
		cat_lines = { it->second };
	}

	void page::process_category(int & cat_font, std::vector<textline *> & cat_lines, int & first_val)
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
				std::vector<BOX*> words = merge_into_words(line->symbols, line->whitespace);
				line->columns = merge_into_columns(words, line->whitespace);
				for (int i = 0; i < line->columns.size(); i++)
					if (line->symbols.size() != 307);
				//	set_border(line->columns[i], 0,0,255);
			}
		}
	}

	void page::determine_columns(std::multimap<int, textline * > & fonts)
	{
		// constants that will be used to determine whitespaces
		int difference = 4;
		int k = 1;

		// iterate over all the lines and create font categories - categories of lines with the same font
		int first_val = fonts.begin()->first;

		// initialize first category
		int cat_font = first_val; // the font if the current category
		std::vector<textline *> cat_lines =  { fonts.begin()->second }; // all the lines that are in the current category
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

	void page::create_table(table & curr_table, std::vector<BOX*> & merged_cols)
	{
		if (!curr_table.textlines.empty())
		{
			// TO DO - create a real table
			curr_table.rows = curr_table.textlines.size();
			curr_table.cols = merged_cols.size();
			curr_table.column_repres = merged_cols;
			curr_table.table_repres = merge_to_table(merged_cols);

			all_tables.push_back(curr_table);

			curr_table = {};
			merged_cols = {};
		}
	}

	std::vector<table> page::merge_cols(std::vector<textline> & page)
	{
		// the resulting vector of all tables in adequate structure
		// vector of textlines that represent one table merged only by columns
		std::vector<BOX*> merged_cols;
		table curr_table;
		for (size_t i = 0; i < page.size() - 1; i++)
		{
			// check two lines that are under each other whether they are in the same table
			std::vector<BOX*> first = page[i].columns;
			if (!merged_cols.empty())
				first = merged_cols;
			std::vector<BOX*> second = page[i + 1].columns;
			// map for saving all the indexes of columns for further merging
			std::map<int, int> no_of_cols;
			// iterators used for iterating over first and second line
			size_t iter_one = 0;
			size_t iter_two = 0;

			// cycle that iterates over columns of the two lines
			while (true)
			{
				// end of cycle
				if (iter_one >= first.size() || iter_two >= second.size())
				{
					// if at least one pair is found, merge
					if (no_of_cols.size() > 0)
					{
						if (curr_table.textlines.empty())
							curr_table.textlines.push_back(&page[i]);
						curr_table.textlines.push_back(&page[i + 1]);
						if (merged_cols.empty())
							merged_cols = merge_lines(page[i].columns, page[i + 1].columns, no_of_cols);
						else merged_cols = merge_lines(merged_cols, page[i + 1].columns, no_of_cols);
					}
					// if there was no match but a table already exists
					else
						create_table(curr_table, merged_cols);
					break;
				}

				// get to the same column in both lines
				if (is_most_left(first[iter_one], second[iter_two])
					&& !are_in_same_col(first[iter_one], second[iter_two]))
				{
					iter_one++;
					continue;
				}
				if (is_most_left(second[iter_two], first[iter_one])
					&& !are_in_same_col(first[iter_one], second[iter_two]))
				{
					iter_two++;
					continue;
				}

				// both are in the same column

				// check whether the chosen columns don't overlap with anything else
				bool to_merge = true;
				for (size_t k = 0; k < first.size(); k++)
				{
					if (k == iter_one)
						continue;
					if (overlap(first[k], second[iter_two]))
						to_merge = false;
				}
				for (size_t k = 0; k < second.size(); k++)
				{
					if (k == iter_two)
						continue;
					if (overlap(second[k], first[iter_one]))
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
		return all_tables;
	}

	table::table()
	{
	}

	void page::delete_footer()
	{
		// TO-DO - calculate footer threshold by something different than a constant
		
		// the last element in the textline vector will be the one that is in the footer

		// deal with multi-line footers
		std::vector<textline>::reverse_iterator it;
		for (it = textlines.rbegin(); it != textlines.rend()-1; it++)
		{
			int line_diff = get_y_axis(it->columns) - get_y_axis(std::next(it)->columns) - get_char_height(std::next(it)->columns, img->w);
			if (line_diff > FOOTER_THRESHOLD)
				break;
		}
		textlines.erase(it.base(), textlines.end());
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
		symbols = {};
		columns = {};
	}

	std::multimap<int, textline *> page::init_textlines()
	{
		// array of symbols that exist in the image
		Boxa * symbol_arr = api->GetComponentImages(tesseract::RIL_SYMBOL, false, NULL, NULL);
		// array of textlines that exist in the image
		Boxa * textline_arr = api->GetComponentImages(tesseract::RIL_TEXTLINE, false, NULL, NULL);

		// sort textline array 
		std::sort(textline_arr->box, textline_arr->box,
			[](BOX* & a, BOX* & b) { return a->y < b->y; });

		BOX* curr_line;
		BOX* curr_symbol;
		textline line;
		textlines = std::vector<textline> (textline_arr->n);

		std::multimap<int, textline*> fonts;

		// iterate over textlines and symbols and initialize all textlines with symbols that belong to them

		for (size_t i = 0; i < textline_arr->n; i++)
		{
			curr_line = textline_arr->box[i];
			for (size_t j = 0; j < symbol_arr->n; j++)
			{
				curr_symbol = symbol_arr->box[j];
				if (is_symbol_in_textline(curr_symbol, curr_line)/* && curr_symbol->w < (img->w / 2) && curr_symbol->w > 5*/ )
					line.symbols.push_back(curr_symbol);
			}
			textlines[i] = line;
			int height = get_char_height(line.symbols, img->w);
			textlines[i].font = height;
			fonts.insert({ height, &textlines[i] });
			line = textline();
		}
		return fonts;
	}

	void page::delete_unusual_lines()
	{
		while (true)
		{
			auto it = std::find_if(textlines.begin(), textlines.end(), [this](textline & line)
			{ return line.whitespace == 0 || line.symbols.empty() /*|| is_textline_table(line)*/;
		});
			if (it != textlines.end())
				textlines.erase(it);
			else
				break;
		}
	}

	BOX * page::merge_to_table(std::vector<BOX*> & cols)
	{
		BOX * result = boxCopy(cols[0]);
		result->w = cols[cols.size() - 1]->x - cols[0]->x + cols[cols.size() - 1]->w;
		return result;
	}

	bool page::is_textline_table(textline & line)
	{
		std::sort(line.symbols.begin(), line.symbols.end(), [](BOX* a, BOX * b) { return a->y < b->y; });
		std::cout << line.symbols.size() << "::" << line.font << "::" << line.symbols.back()->y << "::" << line.symbols[0]->y << std::endl;
		if (line.symbols.back()->y - line.symbols[0]->y > line.font)
			return true;
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
				set_border(all_tables[i].column_repres[j], 255, 0, 0);
			}
			
		}
		
		//std::string out = "results/" + get_filename(filename)+ ".png";
		std::string out = "results/5.png";

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