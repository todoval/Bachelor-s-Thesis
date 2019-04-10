#include "process.h"

namespace ocr
{
	page::page()
	{
	//	page(img);
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

	std::vector<int> page::get_spaces(const std::vector<std::pair<std::unique_ptr<BOX>, std::string>>  & symbols)
	{
		std::vector <int> result;
		// iterate over all symbols and return the whitespaces between them
		auto size = static_cast<int> (symbols.size() - 1);
		for (int i = 0; i < size; i++)
		{
			auto first = &symbols[i];
			auto second = &symbols[i + 1];
			int space = second->first->x - first->first->x - first->first->w;
			result.push_back(space);
		}
		return result;
	}

	std::pair<int, int> page::get_whitespace(std::vector<int> & all_spaces, double constant)
	{
		std::sort(all_spaces.begin(), all_spaces.end());

		std::pair<int, int> result;

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
				return std::make_pair<int, int>(0,0);
			return { all_spaces.back(), img->w };
		}

		// get the word gap

		double multi_cols;

		for (it; it != all_spaces.end() - 1; it++)
		{
			double multi_factor = get_multi_factor_words(*it, constant);
			if (*std::next(it) >= multi_factor * *it)
			{
				// check whether the difference is not already between words and columns
				result.first = *(std::next(it));
				multi_cols = get_multi_factor_columns(*it);
				if (*std::next(it) >= multi_cols * *it)
					return { *(it), *(std::next(it)) };
				it++;
				break;
			}
		}

		if (it == all_spaces.end() - 1)
			return { *(it), *(it) };

		// first result is the word gap and it's in it

		multi_cols = get_multi_factor_columns(result.first);


		for (it; it != all_spaces.end() - 1; it++)
		{
			if (*std::next(it) >= multi_cols * result.first /*3 * *it && *std::next(it) >= 4 * result.first*/)
			{
				result.second = *std::next(it);
				return result;
			}
		}
		
		// second result is the column gap 

		result.second = img->w;
		return result;

	}

	std::vector<std::pair<std::unique_ptr<BOX>, std::string>> page::merge_lines(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & first, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & second, std::map<int, int>& no_of_cols)
	{
		std::vector<std::pair<std::unique_ptr<BOX>, std::string>> result;

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
			auto bbox = boxCopy(first[i].first.get());
			if (bbox == nullptr)
				return {};
			auto new_col = std::pair<std::unique_ptr<BOX>, std::string >{};
			new_col.first = std::unique_ptr<BOX>(bbox);
			new_col.first->h = height;
			new_col.first->y = y;
			// if these columns overlap, add them to the resulting vector
			if (no_of_cols.find(i) != no_of_cols.end())
			{
				int sec_index = no_of_cols[i];
				second_line_indices.erase(std::find(second_line_indices.begin(), second_line_indices.end(), sec_index));

				new_col.first->x = std::min(first[i].first->x, second[sec_index].first->x);
				new_col.first->w = get_width_of_col(first[i].first, second[sec_index].first);
			}
			// if they don't overlap, try to find a suitable column from the second line to merge it with
			else
			{
				bool is_set = false;
				for (auto j : second_line_indices)
				{
					if (overlap(first[i].first, second[j].first))
					{
						// merge these two together
						is_set = true;
						new_col.first->x = std::min(first[i].first->x, second[j].first->x);
						new_col.first->w = get_width_of_col(first[i].first, second[j].first);
						second_line_indices.erase(std::find(second_line_indices.begin(), second_line_indices.end(), j));
						break;
					}
				}
				if (!is_set)
				{
					new_col.first->x = first[i].first->x;
					new_col.first->w = first[i].first->w;
				}
			}
			result.push_back( std::move(new_col));
		}
		// push back second line columns that weren't merged
		for (auto i : second_line_indices)
		{
			auto new_col = std::pair<std::unique_ptr<BOX>, std::string >{};
			new_col.first = std::unique_ptr<BOX>(boxCopy(second[i].first.get()));
			result.push_back(std::move(new_col));
		}
		std::sort(result.begin(), result.end(), [](auto & a, auto & b) {return a.first->x < b.first->x; });
		return std::move(result);
	}

	void page::box_merge_vertical(std::pair<std::unique_ptr<BOX>, std::string> & result, std::pair<std::unique_ptr<BOX>, std::string> & to_add)
	{
		if (result.first == nullptr)
			result = std::move(to_add);
		else
		{
			result.first->w = std::max(result.first->w, to_add.first->w);
			result.first->x = std::min(result.first->x, to_add.first->x);
			result.first->y = std::min(result.first->y, to_add.first->y);
			result.first->h = to_add.first->h + to_add.first->y - result.first->y;
			result.second = result.second + to_add.second;
		}
	}

	void page::box_merge_horizontal(std::pair<std::unique_ptr<BOX>, std::string> & result, std::pair<std::unique_ptr<BOX>, std::string> &to_add)
	{
		if (result.first == nullptr)
			result = std::move(to_add);
		else
		{
			result.first->w = to_add.first->w + to_add.first->x - result.first->x;
			result.first->h = std::max(result.first->h, to_add.first->h);
			result.first->y = std::min(result.first->y, to_add.first->y);
			result.second = result.second + to_add.second;
		}
	}

	std::vector<std::pair<std::unique_ptr<BOX>, std::string>> page::merge_into_words(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & symbols, int whitespace)
	{
		std::sort(symbols.begin(), symbols.end(),
			[](auto & a, auto & b) { return a.first->x < b.first->x; });

 		std::vector<std::pair<std::unique_ptr<BOX>, std::string>> result = {};
		auto word = std::pair<std::unique_ptr<BOX>, std::string >{};
		word.first = std::unique_ptr<BOX>(boxCopy(symbols[0].first.get()));
		word.second = symbols[0].second;

		for (size_t i = 0; i < symbols.size() - 1; i++)
		{
			if (symbols[i].first->x + symbols[i].first->w + whitespace >= symbols[i + 1].first->x)
				box_merge_horizontal(word, symbols[i + 1]);
			else
			{
				result.push_back(std::move(word));
				word.first = std::unique_ptr<BOX>(boxCopy(symbols[i + 1].first.get()));
				word.second = symbols[i + 1].second;
			}
		}
		result.push_back(std::move(word));
		return std::move(result);
	}

	int page::get_column_whitespace(std::vector<int>& word_gaps)
	{
		// get column whitespace
		int column_gap = 0;
		for (auto gap : word_gaps)
			column_gap += gap;
		column_gap /= word_gaps.size();
		
		return 0;
	}

	std::vector<std::pair<std::unique_ptr<BOX>, std::string>> page::merge_into_columns(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & words, int whitespace)
	{
		std::vector<int> word_gaps = get_spaces(words);
		std::vector<std::pair<std::unique_ptr<BOX>, std::string>> columns = {};
		auto column = std::pair<std::unique_ptr<BOX>, std::string > {};
		column.first = std::unique_ptr<BOX>(boxCopy(words[0].first.get()));
		column.second = words[0].second;

		for (size_t j = 0; j < word_gaps.size(); j++)
		{
			if (word_gaps[j] >= whitespace)
			{
				columns.push_back(std::move(column));
				column.first = std::unique_ptr<BOX>(boxCopy(words[j+1].first.get()));
				column.second = words[j + 1].second;
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
			auto whitespaces = get_whitespace(spaces, constant);

			line->word_ws = whitespaces.first;
			line->col_ws = whitespaces.second;
			if (whitespaces.first == whitespaces.second)
				line->word_ws = whitespaces.first;
			else
			{
				int word_ws = whitespaces.first;
				if (word_ws != img->w && word_ws != 0)
					cat_spaces.push_back(word_ws);
			}
		}

		std::sort(cat_spaces.begin(), cat_spaces.end());
		int cat_ws = 0;
		if (!cat_spaces.empty())
			cat_ws = cat_spaces.back();

		for (auto line : cat_lines)
		{
			if (line->symbols.size() <= 1)
				line->word_ws = 0;
			else
			{
				if (line->word_ws == 0)
					line->word_ws = cat_ws;
				auto words = merge_into_words(line->symbols, line->word_ws);
				line->columns = merge_into_columns(words, line->col_ws);
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

	void page::create_table(table & curr_table, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & merged_cols)
	{
		if (!curr_table.textlines.empty())
		{
			// create cells
			
			// vector of textlines that represent one table merged by rows
			std::vector<std::shared_ptr<textline>> curr_row = { curr_table.textlines[0] };
			for (size_t i = 1; i < curr_table.textlines.size(); i++)
			{
				if (are_in_same_row(curr_table.textlines[i-1], curr_table.textlines[i]))
					curr_row.push_back(curr_table.textlines[i]);
				else
				{
					auto bbox = std::unique_ptr<BOX>(boxCopy(curr_row[0]->bbox.get()));
					curr_table.row_repres.push_back(std::move(bbox));

					// create cells

					auto cells = create_cells(curr_row, merged_cols);
					move_append(cells, curr_table.cells);

					curr_row = { curr_table.textlines[i] };
				}
			}
			auto cells = create_cells(curr_row, merged_cols);
			move_append(cells, curr_table.cells);

			curr_table.rows = curr_table.row_repres.size();
			curr_table.cols = merged_cols.size();
			curr_table.table_repres = merge_to_table(merged_cols);
			curr_table.column_repres = std::move(merged_cols);

			all_tables.push_back(std::move(curr_table));

			merged_cols.clear();
		}
	}

	std::vector<cell> page::create_cells(std::vector<std::shared_ptr<textline>> & row, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & merged_cols)
	{
		std::vector<cell> result;
		for (size_t j = 0; j < merged_cols.size(); j++)
		{
			cell curr_cell;
			curr_cell.bbox = std::unique_ptr<BOX>(boxCopy(merged_cols[j].first.get()));
			if (curr_cell.bbox == nullptr)
			{
				merged_cols.clear();
				continue;
			}
			curr_cell.bbox->h = get_merged_lines_height(row);
			curr_cell.bbox->y = get_y_axis(row[0]->symbols);
			curr_cell.cols_no = 1;
			curr_cell.rows_no = row.size();
			//curr_cell.text = 
			
			result.push_back(std::move(curr_cell));
		}
		row.clear();
		return std::move(result);
	}

	void page:: move_append(std::vector<cell>& source, std::vector<cell>& dest)
	{
		if (dest.empty())
			dest = std::move(source);
		else
		{
			dest.reserve(dest.size() + source.size());
			std::move(std::begin(source), std::end(source), std::back_inserter(dest));
			source.clear();
		}
	}

	std::vector<table> page::merge_cols(std::vector<std::shared_ptr<textline>> & page)
	{
		// the resulting vector of all tables in adequate structure
		// vector of textlines that represent one table merged by columns
		std::vector<std::pair<std::unique_ptr<BOX>, std::vector<std::string>>> merged_cols;
		table curr_table;
		for (size_t i = 0; i < page.size() - 1; i++)
		{
			// don't merge lines that are too far away from each other

			if (get_y_axis(page[i + 1]->symbols) - get_y_axis(page[i]->symbols) > 4*std::max(get_greatest_font(page[i + 1]->symbols), get_greatest_font(page[i]->symbols)))
			{
				if (merged_cols.size() > 0)
					create_table(curr_table, merged_cols);
				continue;
			}

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
				if (is_most_left(first->at(iter_one).first, second->at(iter_two).first)
					&& !are_in_same_col(first->at(iter_one).first, second->at(iter_two).first))
				{
					iter_one++;
					continue;
				}
				if (is_most_left(second->at(iter_two).first, first->at(iter_one).first)
					&& !are_in_same_col(first->at(iter_one).first, second->at(iter_two).first))
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
					if (overlap(first->at(k).first, second->at(iter_two).first))
						to_merge = false;
				}
				for (size_t k = 0; k < second->size(); k++)
				{
					if (k == iter_two)
						continue;
					if (overlap(second->at(k).first, first->at(iter_one).first))
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

		// sort textlines by y coordinate
		std::sort(textlines.begin(), textlines.end(), [](auto & a, auto & b) { return a->symbols[0].first->y < b->symbols[0].first->y;  });

		// check whether footer even exists
		if (textlines.back()->symbols[0].first->y + textlines.back()->symbols[0].first->h < img->h - img->h / 10)
			return;

		std::vector<std::shared_ptr<textline>>::reverse_iterator it = textlines.rbegin();
		int no_of_lines = 0;
		for (it = textlines.rbegin(); it != textlines.rend() - 1 ; it++)
		{
			int line_diff = get_y_axis(it->get()->columns) - get_y_axis(std::next(it)->get()->columns) - get_char_height(std::next(it)->get()->columns, img->w);
			if (line_diff > FOOTER_THRESHOLD)
				break;
			no_of_lines++;
			// footer isn't greater than 10 textlines
			if (no_of_lines == 10)
				return;
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
		std::shared_ptr<textline> line = std::make_shared<textline>();
		textlines = std::vector<std::shared_ptr<textline>>(textline_arr->n);

		std::multimap<int, std::shared_ptr<textline>> fonts;

		// iterate over textlines and symbols and initialize all textlines with symbols that belong to them

		for (size_t i = 0; i < textline_arr->n; i++)
		{
			curr_line = std::unique_ptr<BOX>(boxCopy(textline_arr->box[i]));

			tesseract::ResultIterator* ri = api->GetIterator();
			tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
			if (ri != 0)
			{
				do
				{
					const char* word = ri->GetUTF8Text(level);
					bool del = false;
					for (size_t i = 0; word[i] != 0; i++)
					{
						if ((unsigned char)word[i] == ' ')
							del = true;
					}
					if (del)
					{
						delete[] word;
						continue;
					}

					float conf = ri->Confidence(level);
					int x1, y1, x2, y2;
					ri->BoundingBox(level, &x1, &y1, &x2, &y2);
					auto new_box = std::unique_ptr<BOX>(boxCreate(x1, y1, x2 - x1, y2 - y1));
					if (is_symbol_in_textline(new_box, curr_line))
					{
						std::string sym(word);
						line->symbols.push_back({ std::move(new_box) , sym });
					}
						
					delete[] word;
				} while (ri->Next(level));
			}
			int height = get_char_height(line->symbols, img->w);
			line->bbox = std::move(curr_line);
			textlines[i] = line;
			textlines[i]->font = height;
			fonts.insert({ height, textlines[i] });
			line = std::make_shared<textline>();
		}
		delete[] textline_arr;

		return fonts;
	}

	void page::delete_unusual_lines()
	{
		while (true)
		{
			auto it = std::find_if(textlines.begin(), textlines.end(), [this](auto line)
			{ return line->word_ws == 0 || line->symbols.empty() || is_textline_table(line);
			});
			if (it != textlines.end())
				textlines.erase(it);
			else
				break;
		}
	}

	std::unique_ptr<BOX> page::merge_to_table(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & cols)
	{
		if (cols.empty())
			return nullptr;
		auto result = std::unique_ptr<BOX>(boxCopy(cols[0].first.get()));
		if (result == nullptr)
			return nullptr;
		if (cols.size() > 1)
			result->w = cols[cols.size() - 1].first->x - cols[0].first->x + cols[cols.size() - 1].first->w;
		return std::move(result);
	}

	bool page::is_textline_table(std::shared_ptr<textline> line)
	{
		int y_lowest = get_y_axis(line->symbols) + line->font*2;
		for (auto & symbol : line->symbols)
		{
			if (symbol.first->y > y_lowest)
				return true;
		}
		return false;
	}

	bool page::are_in_same_row(std::shared_ptr<textline>& first, std::shared_ptr<textline>& second)
	{
		int first_size = first->columns.size();
		int sec_size = second->columns.size();
		if (first_size == sec_size)
			return false;
		int max = std::max(first_size, sec_size);


		for (auto & first_col : first->columns)
		{
			for (auto & second_col : second->columns)
			{
				if (overlap(first_col.first, second_col.first));
					//return true;
			}
		}
		return false;
	}

	int page::get_merged_lines_height(std::vector<std::shared_ptr<textline>>& lines)
	{
		int heighest = 0;
		for (auto & symbol : lines.back()->symbols)
		{
			if (symbol.first->y + symbol.first->h > heighest)
				heighest = symbol.first->y + symbol.first->h;
		}
		int lowest = get_y_axis(lines.front()->symbols);
		return heighest - lowest;
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

		for (auto k : textlines)
		{
			for (auto & i : k->columns);
		//	set_border(i.first, 0, 0, 255);
		}

		all_tables = merge_cols(textlines);

		for (int i = 0; i < all_tables.size(); i++)
		{
			for (int j = 0; j < all_tables[i].cells.size(); j++)
			{
				if (all_tables[i].column_repres.size() > 1)
					//set_border(all_tables[i].column_repres[j].first, 0, 0, 255);

					set_border(all_tables[i].cells[j].bbox, 120, 120, 255);
				//	set_border(all_tables[i].table_repres, 0, 0, 255);
			}

		}

		std::string out = "results/" + get_filename(filename) + "-bin.png"; 

		char* path = &out[0u];
		pixWrite(path, img, IFF_PNG);
		api->End();
		pixDestroy(&img);

	}

	cell::cell()
	{
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