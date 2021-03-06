#include "process.h"

using namespace tabular_ocr;
using namespace tabular_ocr::ocr;

page::page(file_info & file)
{
	filename = file.name;
	// itialize tesseract api without the use of LSTM
	api = std::make_unique<tesseract::TessBaseAPI>();
	img_old = std::move(file.old);
	if (file.preprocessed->d == 8)
		file.preprocessed = image(pixConvert8To32(file.preprocessed.get()));
	img_preprocessed = std::move(file.preprocessed);
	init_api(img_preprocessed);
}

void page::set_border(const bbox & box, int r, int g, int b)
{
	for (int i = 0; i < box.w; i++)
	{
		pixSetRGBPixel(img_old.get(), box.x + i, box.y, r, g, b);
		pixSetRGBPixel(img_old.get(), box.x + i, box.y + box.h, r, g, b);
	}
	for (int i = 0; i < box.h; i++)
	{
		pixSetRGBPixel(img_old.get(), box.x, box.y + i, r, g, b);
		pixSetRGBPixel(img_old.get(), box.x + box.w, box.y + i, r, g, b);
	}
}

bool page::is_symbol_in_textline(bbox & symbol, bbox & textline)
{
	return (symbol.x <= textline.x + textline.w
		&& symbol.x >= textline.x
		&& symbol.y >= textline.y
		&& symbol.x + symbol.w <= textline.x + textline.w
		&& symbol.y + symbol.h <= textline.y + textline.h);
}

std::vector<int> page::get_spaces(const std::vector<boxed_string> & symbols)
{
	std::vector <int> result;
	// iterate over all symbols and return the whitespaces between them
	auto size = static_cast<int> (symbols.size() - 1);
	for (int i = 0; i < size; i++)
	{
		auto first = &symbols[i];
		auto second = &symbols[i + 1];
		int space = second->box.x - first->box.x - first->box.w;
		result.push_back(space);
	}
	return result;
}

std::vector<int>::iterator page::get_val_above_constant (std::vector<int> & all_spaces, double constant)
{
	auto it = all_spaces.begin();
	for (it = all_spaces.begin(); it != all_spaces.end(); it++)
	{
		if (*it > std::max(constant, 1.0))
			break;
	}
	return it;
}

bool page::rows_in_different_tables(const textline& first, const textline& second)
{
	return get_y_axis(second.symbols) - get_y_axis(first.symbols) > ROW_CONSTANT*std::max(second.font, first.font);
}

bool page::find_same_column(size_t & iter_one, size_t & iter_two, std::vector<boxed_string> * first, std::vector<boxed_string> * second)
{
	// get to the same column in both lines
	if (is_most_left(first->at(iter_one).box, second->at(iter_two).box)
		&& !are_in_same_col(first->at(iter_one).box, second->at(iter_two).box))
	{
		iter_one++;
		return false;
	}
	if (is_most_left(second->at(iter_two).box, first->at(iter_one).box)
		&& !are_in_same_col(first->at(iter_one).box, second->at(iter_two).box))
	{
		iter_two++;
		return false;
	}
	return true;
}

bool page::can_merge_columns(size_t & iter_one, size_t & iter_two, std::vector<boxed_string>* first, std::vector<boxed_string>* second)
{
	for (size_t k = 0; k < first->size(); k++)
	{
		if (k == iter_one)
			continue;
		if (overlap(first->at(k).box, second->at(iter_two).box))
			return false;
	}
	for (size_t k = 0; k < second->size(); k++)
	{
		if (k == iter_two)
			continue;
		if (overlap(second->at(k).box, first->at(iter_one).box))
			return false;
	}
	return true;
}

std::pair<int, int> page::get_whitespaces(std::vector<int> & all_spaces, double constant)
{
	// check whether there even exist spaces
	if (all_spaces.empty())
		return std::make_pair<int, int>(0, 0);

	// sort spaces and start the algorithm
	std::sort(all_spaces.begin(), all_spaces.end());

	std::pair<int, int> result;


	// iterate the all_spaces vector to get above the constant value
	auto it = get_val_above_constant(all_spaces, constant);

	// if there was no value higher than the constant, determine the word ws to be the highest value and the column ws none
	if (it == all_spaces.end())
		return { all_spaces.back(), img_preprocessed->w };

	double multi_cols; // multiplication factor of columns

	// get word whitespace
	for (it; it != all_spaces.end() - 1; it++)
	{
		// get multiplication factor of current space
		double multi_factor = get_multi_factor_words(*it, constant);
		if (*std::next(it) >= multi_factor * *it)
		{
			// found the word whitespace

			// check whether the difference found is not between words and columns
			multi_cols = get_multi_factor_columns(*it);
			if (*std::next(it) >= multi_cols * *it)
				return { *(it), *(std::next(it)) };

			// if no than the difference was between words
			result.first = *(std::next(it));
			it++;
			break;
		}
	}

	if (it == all_spaces.end() - 1)
		return { *(it), *(it) };

	// calculate the column whitespace using the word whitespace

	multi_cols = get_multi_factor_columns(result.first);
	for (it; it != all_spaces.end() - 1; it++)
	{
		if (*std::next(it) >= multi_cols * result.first)
		{
			// found the column whitespace
			result.second = *std::next(it);
			return result;
		}
	}
		
	// if no column whitespace was found, determine the line has no columns
	result.second = img_preprocessed->w;
	return result;
}

std::vector<boxed_string> page::merge_lines(const std::vector<boxed_string> & first, const std::vector<boxed_string> & second, std::map<int, int>& no_of_cols)
{
	std::vector<boxed_string> result;

	// calculate the y axis and height of the new merged boxes

	int y = get_y_axis(first);
	int height = get_char_height(second, img_preprocessed->w) + get_y_axis(second) - y;

	// used to keep track of already merged columns from second line
	std::vector<size_t> second_line_indices;
	for (size_t i = 0; i < second.size(); i++)
		second_line_indices.push_back(i);

	// go over the first vector and either add from given map or at only this box into the new vector
	for (size_t i = 0; i < first.size(); i++)
	{
		auto bbox = first[i].box;
		if (bbox.not_initialized())
			return {};
		auto new_col = boxed_string{};
		new_col.box = bbox;
		new_col.box.h = height;
		new_col.box.y = y;
		// if current column has a pair that it should be merge with, merge
		if (no_of_cols.find(i) != no_of_cols.end())
		{
			int sec_index = no_of_cols[i];
			second_line_indices.erase(std::find(second_line_indices.begin(), second_line_indices.end(), sec_index));
			new_col.box.x = std::min(first[i].box.x, second[sec_index].box.x);
			new_col.box.w = get_width_of_col(first[i].box, second[sec_index].box);
		}
		// otherwise try to find a suitable column from the second line to merge it with
		else
		{
			bool is_set = false;
			for (auto j : second_line_indices)
			{
				if (overlap(first[i].box, second[j].box))
				{
					// merge these two together
					is_set = true;
					new_col.box.x = std::min(first[i].box.x, second[j].box.x);
					new_col.box.w = get_width_of_col(first[i].box, second[j].box);
					second_line_indices.erase(std::find(second_line_indices.begin(), second_line_indices.end(), j));
					break;
				}
			}
			if (!is_set)
			{
				new_col.box.x = first[i].box.x;
				new_col.box.w = first[i].box.w;
			}
		}
		result.push_back(new_col);
	}
	// push back second line columns that weren't merged

	for (auto i : second_line_indices)
	{
		auto new_col = boxed_string {};
		new_col.box = second[i].box;
		result.push_back(new_col);
	}
	sort_by_xcoord(result);
	return result;
}

void page::box_merge_vertical(boxed_string & result, const boxed_string & to_add)
{
	if (result.box.not_initialized())
		result = to_add;
	else
	{
		result.box.w = std::max(result.box.w, to_add.box.w);
		result.box.x = std::min(result.box.x, to_add.box.x);
		result.box.y = std::min(result.box.y, to_add.box.y);
		result.box.h = to_add.box.h + to_add.box.y - result.box.y;
		result.text = result.text + to_add.text;
	}
}

void page::box_merge_horizontal(boxed_string & result, const boxed_string &to_add, bool add_space)
{
	if (result.box.not_initialized())
		result = to_add;
	else
	{
		result.box.w = to_add.box.w + to_add.box.x - result.box.x;
		result.box.h = std::max(result.box.h, to_add.box.h);
		result.box.y = std::min(result.box.y, to_add.box.y);
		if (add_space)
			result.text = result.text + " " + to_add.text;
		else
			result.text = result.text + to_add.text;
	}
}

std::vector<boxed_string> page::merge_into_words(const std::vector<boxed_string> & symbols, int whitespace)
{
 	std::vector<boxed_string> result = {};
	auto word = boxed_string{};
	word.box = symbols[0].box;
	word.text = symbols[0].text;
	for (size_t i = 0; i < symbols.size() - 1; i++)
	{
		if (symbols[i].box.x + symbols[i].box.w + whitespace >= symbols[i + 1].box.x)
			box_merge_horizontal(word, symbols[i + 1], false);
		else
		{
			result.push_back(word);
			word.box = symbols[i + 1].box;
			word.text = symbols[i + 1].text;
		}
	}
	result.push_back(word);
	return result;
}

std::vector<boxed_string> page::merge_into_columns(const std::vector<boxed_string> & words, int whitespace)
{
	std::vector<int> word_gaps = get_spaces(words);
	std::vector<boxed_string> columns = {};
	auto column = boxed_string {};
	column.box = words[0].box;
	column.text = words[0].text;

	for (size_t j = 0; j < word_gaps.size(); j++)
	{
		if (word_gaps[j] >= whitespace)
		{
			columns.push_back(column);
			column.box = words[j+1].box;
			column.text = words[j + 1].text;
		}
		else
			box_merge_horizontal(column, words[j + 1], true);
	}
	columns.push_back(column);
	return columns;
}

void page::determine_columns()
{
	for (auto & line : textlines)
	{
		// calculate constant that means the minimum required whitespace between words
		double constant = line.font / REF_FONT_DIVIDER;
		auto spaces = get_spaces(line.symbols);
		auto whitespaces = get_whitespaces(spaces, constant);

		line.word_ws = whitespaces.first;
		line.col_ws = whitespaces.second;

		// if line is unusual, determine that it will only have one word and column
		if (line.symbols.size() <= 1)
			line.word_ws = img_preprocessed->w;

		// initialize columns of textline by merging
		sort_by_xcoord(line.symbols);
		auto words = merge_into_words(line.symbols, line.word_ws);
		line.columns = merge_into_columns(words, line.col_ws);
	}
}

std::vector<bbox> page::remove_string_from_pair(const std::vector<boxed_string> & input)
{
	std::vector<bbox> result;
	for (auto & elem : input)
		result.push_back(elem.box);
	return result;
}

bool page::is_word_empty(const std::string& word)
{
	for (auto c:word)
	{
		if (c != ' ') return false;
	}
	return true;
}

int page::get_row_whitespace(const std::vector<textline> lines)
{
	std::vector<int> whitespaces;
	for (size_t i = 0; i < lines.size() - 1; i++)
		whitespaces.push_back(lines[i + 1].box.y - (lines[i].box.h + lines[i].box.y));
	std::sort(whitespaces.begin(), whitespaces.end());
	std::pair<int, double> whitespace = { 0,0 }; // whitespace and its multiplicator
	for (size_t i = 0; i < whitespaces.size() - 1; i++)
	{
		if (whitespaces[i] == 0)
			continue;
		double multi = whitespaces[i + 1] / whitespaces[i];
		if (multi > 2.5 && multi > whitespace.second)
		{
			whitespace.first = whitespaces[i];
			whitespace.second = multi;
		}
	}
	return whitespace.first;
}

void page::set_table_borders()
{
	for (size_t i = 0; i < all_tables.size(); i++)
		set_cell_borders(all_tables[i]);
}

void page::set_cell_borders(const table & table)
{
	for (auto & cell : table.cells)
		set_border(cell.box, 255, 0, 0);
}

void tabular_ocr::ocr::page::end_api()
{
	// end and clear api to avoid memory leaks
	api->End();
	api->ClearPersistentCache();
	api->Clear();
}

void page::init_table(table & curr_table, std::vector<boxed_string> & merged_cols)
{
	if (!curr_table.textlines.empty() && merged_cols.size() > 1)
	{
		// create cells
			
		// vector of textlines that represent one table merged by rows
		std::vector<textline> curr_row = { curr_table.textlines[0] };
		size_t row_no = 0;

		// whitespace that determines whether cells are in the same row or not
		int row_ws = get_row_whitespace(curr_table.textlines);

		for (size_t i = 1; i < curr_table.textlines.size(); i++)
		{
			if (are_in_same_row(curr_table.textlines[i-1], curr_table.textlines[i], row_ws))
				curr_row.push_back(curr_table.textlines[i]);
			else
			{
				auto bbox = curr_row[0].box;
				curr_table.row_repres.push_back(bbox);

				// create cells

				auto cells = create_cells(curr_row, merged_cols, row_no);
				row_no++;
				move_append(curr_table.cells, cells);
				curr_row = { curr_table.textlines[i] };
			}
		}
		auto cells = create_cells(curr_row, merged_cols, row_no);
		move_append(curr_table.cells, cells);

		curr_table.rows = curr_table.row_repres.size();
		curr_table.cols = merged_cols.size();
		curr_table.table_repres = std::move(merge_to_table_box(merged_cols));
		curr_table.column_repres = std::move(remove_string_from_pair(merged_cols));

		all_tables.push_back(std::move(curr_table));
	}

	merged_cols.clear();
	curr_table.textlines.clear();
}

std::vector<cell> page::create_cells(std::vector<textline> & row, std::vector<boxed_string> & merged_cols, size_t row_no)
{
	std::vector<cell> result;
	for (size_t j = 0; j < merged_cols.size(); j++)
	{
		cell curr_cell;
		curr_cell.box = merged_cols[j].box;
		if (curr_cell.box.not_initialized())
		{
			merged_cols.clear();
			continue;
		}
		curr_cell.box.h = get_merged_lines_height(row);
		curr_cell.box.y = get_y_axis(row[0].symbols);
		curr_cell.cols_no = j;
		curr_cell.rows_no = row_no;
		for (auto line : row)
			for (auto & col : line.columns)
				if (overlap(col.box, curr_cell.box))
					curr_cell.text = col.text + " ";
			
		result.push_back(std::move(curr_cell));
	}
	row.clear();
	return std::move(result);
}

void page::move_append(std::vector<cell>& dest, std::vector<cell>& source)
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

void page::create_tables_from_cols()
{
	if (textlines.size() <= 1)
		return;
	// vector of boxes that represent current table merged by columns
	std::vector<boxed_string> merged_cols;
	table curr_table;
	for (size_t i = 0; i < textlines.size() - 1; i++)
	{
		// don't merge lines that are too far away from each other
		if (rows_in_different_tables(textlines[i], textlines[i+1]))
		{
			if (merged_cols.size() > 0)
			//curr_table.textlines.push_back(textlines[i]);
				init_table(curr_table, merged_cols);
			continue;
		}

		// initialize two lines that are under each other (or use an already existing table)
		auto first = &textlines[i].columns;
		if (!merged_cols.empty())
			first = &merged_cols;
		auto second = &textlines[i + 1].columns;

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
					// add textlines
					if (curr_table.textlines.empty())
						curr_table.textlines.push_back(textlines[i]);
					curr_table.textlines.push_back(textlines[i + 1]);
					// add current line to an already existing table or initialize table
					if (merged_cols.empty())
						merged_cols = merge_lines(textlines[i].columns, textlines[i + 1].columns, no_of_cols);
					else merged_cols = merge_lines(merged_cols, textlines[i + 1].columns, no_of_cols);
				}
				// if there was no match but a table already exists
				else
					init_table(curr_table, merged_cols);
				break;
			}

			// get to the same column in both lines
			if (!find_same_column(iter_one, iter_two, first, second))
				continue;

			// both iter_one and iter_two are now in the same column

			// check whether the chosen columns don't overlap with anything else
			if (can_merge_columns(iter_one, iter_two, first, second))
				no_of_cols.insert(std::pair<int, int>(iter_one, iter_two));
			else
			{
				// not going to merge but table may already exist
				init_table(curr_table, merged_cols);
				no_of_cols.clear();
				break;
			}
			iter_two++;
			iter_one++;
		}
	}
	init_table(curr_table, merged_cols);
}

void page::delete_footer()
{
	if (textlines.empty())
		return;

	// check whether footer even exists
	if (textlines.back().symbols[0].box.y + textlines.back().symbols[0].box.h < img_preprocessed->h - img_preprocessed->h / 10)
		return;

	int no_of_lines = 0;
	// iterate from the end of the page and check for footers
	std::vector<textline>::reverse_iterator it = textlines.rbegin();
	for (it; it != textlines.rend() - 1 ; it++)
	{
		// calculate the height difference between two textlines above each other
		auto k = *(std::next(it));
		int line_diff = get_y_axis((*it).columns) - get_y_axis((*std::next(it)).columns) - (*std::next(it)).font;
		if (line_diff > FOOTER_THRESHOLD)
			break;
		no_of_lines++;
		// footer size can't be greater than 5 textlines
		if (no_of_lines == 5)
			return;
	}
	// erase the footer
	auto forward_it = (it + 1).base();
	textlines.erase(forward_it, textlines.end());
}

void page::init_api(image &img)
{
	if (api->Init(NULL, "eng", tesseract::OcrEngineMode::OEM_TESSERACT_ONLY))
	{
		// fatal error
		std::cerr << "Failed to initialize tesseract!" << std::endl;
		exit(1);
	}
	api->SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);
	api->SetImage(img.get());
	//	api->SetVariable("user_defined_dpi", "72");
	api->Recognize(0);
}

void page::init_textlines()
{
	// array of textlines that exist in the image
	std::vector<bbox> textline_arr;
	{
		Boxa * ba = api->GetComponentImages(tesseract::RIL_TEXTLINE, false, NULL, NULL);
		textline_arr.reserve(ba->n);
		for(auto i=0;i<ba->n;++i) {
			textline_arr.emplace_back(ba->box[i]);
		}
		boxaDestroy(&ba);
	}

	// sort textline array by their y-coordinate
	std::sort(textline_arr.begin(), textline_arr.end(),
		[](const bbox&a, const bbox&b) { return a.y < b.y; });

	// represents current textline as a box structure
	bbox curr_line;
	// represents the textline that will be added to the textline vector
	textline line;

	// initialize the textline vector that is a part of the page class
	textlines.clear();
	textlines.resize(textline_arr.size());

	// iterate over textlines and symbols and initialize all textlines with symbols that belong to them
	for (size_t i = 0; i < textline_arr.size(); i++)
	{
		curr_line = textline_arr[i];

		// get the tesseract symbol iterator
		std::unique_ptr<tesseract::ResultIterator> ri(api->GetIterator());
		tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
		if (ri)
		{
			do
			{
				std::string sym;
				std::unique_ptr<const char[]> word(ri->GetUTF8Text(level));
				sym.assign(word.get());

				// check whether the given word is not a false positive symbol
				if (is_word_empty(sym)) continue;
				float conf = ri->Confidence(level);

				// get the bbox of the recognized symbol
				bbox sym_box;
				int x1, y1, x2, y2;
				ri->BoundingBox(level, &x1, &y1, &x2, &y2);
				sym_box = bbox(x1, y1, x2 - x1, y2 - y1);

				if (is_symbol_in_textline(sym_box, curr_line))
					line.symbols.push_back({sym_box, sym});
			} while (ri->Next(level));
		}

		// process other parameters of the textline and add this line to the textline vector

		int height = get_char_height(line.symbols, img_preprocessed->w);
		line.box = curr_line;
		textlines[i] = line;
		textlines[i].font = height;
		line = textline();
	}
}

void page::delete_unusual_lines()
{
	std::vector<textline> filtered_lines;
	for (auto & line : textlines)
	{
		if (!line.symbols.empty() && !is_textline_table(line))
			filtered_lines.push_back(line);
	}
	std::swap(filtered_lines, textlines);
}

bbox page::merge_to_table_box(const std::vector<boxed_string> & cols)
{
	if (cols.empty())
		return bbox();
	auto result = cols[0].box;
	if (result.not_initialized())
		return bbox();
	if (cols.size() > 1)
		result.w = cols[cols.size() - 1].box.x - cols[0].box.x + cols[cols.size() - 1].box.w;
	return result;
}

bool page::is_textline_table(const textline & line)
{
	int y_lowest = get_y_axis(line.symbols) + line.font*2;
	for (auto & symbol : line.symbols)
	{
		if (symbol.box.y > y_lowest)
			return true;
	}
	return false;
}

bool page::are_in_same_row(const textline& first, const textline& second, int whitespace)
{
	if (first.box.y + first.box.h + whitespace > second.box.y)
		return true;
	return false;
}

int page::get_merged_lines_height(const std::vector<textline>& lines)
{
	int heighest = 0;
	for (auto & symbol : lines.back().symbols)
	{
		if (symbol.box.y + symbol.box.h > heighest)
			heighest = symbol.box.y + symbol.box.h;
	}
	int lowest = get_y_axis(lines.front().symbols);
	return heighest - lowest;
}

void page::process_image()
{

	int spp = pixGetSpp(img_old.get());
	if (spp == 4)
	{
		auto rem_img = image(pixRemoveAlpha(img_old.get()));
		pixSetSpp(rem_img.get(), 3);
		img_old = image(pixCopy(nullptr, rem_img.get()));
	}

	if (img_old.get() == NULL)
	{
		std::cerr << "Error: input image size too big" << std::endl;
		end_api();
		exit(1);
	}

	// initializes the textlines vector - adds symbols and fonts to the existing textlines
	init_textlines();

	delete_unusual_lines();

	// save information about columns into the textline vector
	determine_columns();

	delete_footer();

	// merge columns into tables

	create_tables_from_cols();

	end_api();
}

json table::to_json()
{
	json cell_list = json::array();
	for (auto & cell : cells)
	{
		cell_list.push_back(cell.to_json());
	}
	return json{
		{"rows",rows},
		{"cols",cols},
		{"table_repres",table_repres.to_json()},
		{"cells",cell_list}
	};
}

json page::to_json()
{
	json table_list = json::array();
	for (auto & table : all_tables)
		table_list.push_back(table.to_json());
	return table_list;
}

json cell::to_json()
{
	return json{
		{"text",text},
		{"box",box.to_json()},
		{"rows_no",rows_no},
		{"cols_no",cols_no}
	};
}