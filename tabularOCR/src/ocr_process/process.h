#include <vector>
#include <algorithm> 
#include "utils.h"
#include <memory>
#include <unordered_map>

namespace ocr
{

	const int FOOTER_THRESHOLD = 30;

	const double REF_FONT_DIVIDER = 16;

	const double ROW_CONSTANT = 4.325;

	class textline
	{
	public:
		size_t font;
		size_t word_ws;
		size_t col_ws;
		std::unique_ptr<BOX> bbox;
		std::vector<std::pair<std::unique_ptr<BOX>, std::string>> symbols;
		std::vector<std::pair<std::unique_ptr<BOX>, std::string>> columns;

		textline(const textline &) = delete;
		textline & operator=(const textline&) = delete;
		textline& operator=(textline&& other)
		{
			font = std::move(other.font);
			word_ws = std::move(other.word_ws);
			col_ws = std::move(other.col_ws);
			symbols = std::move(other.symbols);
			columns = std::move(other.columns);
			bbox = std::move(other.bbox);
			return *this;
		}

		textline();

		bool operator==(const textline & other) const
		{
			return (other.col_ws == col_ws && other.word_ws == word_ws && font == other.font && bbox == other.bbox
				&& symbols.size() == other.symbols.size() && columns.size() == other.columns.size());
		}
	};

	class cell
	{
	public:
		std::string text;
		std::unique_ptr<BOX> bbox;
		int rows_no;
		int cols_no;

		cell();

		cell(const cell &) = delete;
		cell & operator=(const cell&) = delete;
		cell& operator=(cell&& other)
		{
			text = std::move(other.text);
			bbox = std::move(other.bbox);
			rows_no = std::move(other.rows_no);
			cols_no = std::move(other.cols_no);
			return *this;
		}
		cell(cell && other)
		{
			text = std::move(other.text);
			bbox = std::move(other.bbox);
			rows_no = std::move(other.rows_no);
			cols_no = std::move(other.cols_no);
		}

	};

	class table
	{
	public:
		size_t rows;
		size_t cols;

		std::vector<std::unique_ptr<BOX>> row_repres;
		std::vector<std::unique_ptr<BOX>> column_repres;
		std::unique_ptr<BOX> table_repres;
		std::vector<cell> cells;

		std::vector<std::shared_ptr<textline>> textlines;
		
		table(const table &) = delete;
		table(table && other)
		{
			rows = std::move(other.rows);
			cols = std::move(other.cols);
			row_repres = std::move(other.row_repres);
			column_repres = std::move(other.column_repres);
			table_repres = std::move(other.table_repres);
			textlines = std::move(other.textlines);
			cells = std::move(other.cells);
		}
		table & operator=(const table&) = delete;
		table& operator=(table&& other)
		{
			rows = std::move(other.rows);
			cols = std::move(other.cols);
			row_repres = std::move(other.row_repres);
			column_repres = std::move(other.column_repres);
			table_repres = std::move(other.table_repres);
			textlines = std::move(other.textlines);
			return *this;
		}

		table();
	};

	class page
	{
	public:
		Pix *img_old;
		Pix *img_preprocessed;
		std::vector<table> all_tables;
		std::vector<std::shared_ptr<textline>> textlines;

		page(const page &) = delete;
		page & operator=(const page&) = delete;
		page();
		page(preprocessing::file_info file);
		~page()
		{
			
		};

		// colors the border of the given box in the current image with the given rgb color
		void set_border(std::unique_ptr<BOX> & box, int r, int g, int b);
		// the main function
		void process_image();

	private:
		tesseract::TessBaseAPI *api;
		std::string filename;

		// assigns each textline it's columns and whitespaces
		void determine_columns();

		// initializes tesseract api without the use of LSTM
		void init_api(Pix *img);

		// gets the symbols and lines recognized by tesseract and initializes class's textlines vector with recognized values
		void init_textlines();

		// returns true if a symbol (defined by a box) is in textline (defined by another box)
		bool is_symbol_in_textline(std::unique_ptr<BOX> & symbol, std::unique_ptr<BOX> & textline);

		// if a footer on the page exists, it removes it
		void delete_footer();

		// merges two given boxes horizontally and returns the result in the result parameter
		void box_merge_horizontal(std::pair<std::unique_ptr<BOX>, std::string> & result, std::pair<std::unique_ptr<BOX>, std::string> & to_add);

		// merges two given boxes vertically and returns the result in the result parameter
		void box_merge_vertical(std::pair<std::unique_ptr<BOX>, std::string> & result, std::pair<std::unique_ptr<BOX>, std::string> & to_add);

		//merges columns into tables and saves them into the all_tables vector 
		void create_tables_from_cols(std::vector<std::shared_ptr <textline>> & page);
		
		// returns a merged vector of words given a vector of symbols and whitespace between words in a single textline
		std::vector<std::pair<std::unique_ptr<BOX>, std::string>> merge_into_words(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & symbols, int whitespace);
		
		// returns a merged vector of columns given a vector of words and whitespace between columns in a single textline
		std::vector<std::pair<std::unique_ptr<BOX>, std::string>> page::merge_into_columns(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & words, int whitespace);

		// returns a vector of whitespaces that exist between given symbols - should not be used on multiline textlines
		std::vector<int> get_spaces(const std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & symbols);

		// returns a pair representing whitespace between words and whitespace between columns given the spaces in a textline
		std::pair<int, int> get_whitespaces(std::vector<int> & all_spaces, double constant);

		// merges two given lines according to given map that tells which pairs of boxes should be merged
		std::vector<std::pair<std::unique_ptr<BOX>, std::string>> merge_lines(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & first, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & second, std::map<int,int> & no_of_cols);

		// removes textlines that are unimportant for recognition from the textline vector
		void delete_unusual_lines();

		// returns a box representation of a table
		std::unique_ptr<BOX> merge_to_table_box(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & cols);

		// initialize the given table with the information given by the second parameter
		void init_table(table & curr_table, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & merged_cols);

		// returns true if the line recognized by tesseract is too big to be a simple textline - probably a badly recognized table
		bool is_textline_table(std::shared_ptr<textline> line);

		// returns true if given textlines from recognized table are close enough to be in a same cell together
		bool are_in_same_row(std::shared_ptr<textline> & first, std::shared_ptr<textline> & second, int whitespace);

		// returns the vector of cells in a table defined by rows and columns
		std::vector<cell> create_cells(std::vector<std::shared_ptr<textline>> & row, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & merged_cols);

		// appends the source vector to the dest vector
		void move_append(std::vector<cell>& dest, std::vector<cell>& source);

		// returns the height of the input textlines merged together
		int get_merged_lines_height(std::vector<std::shared_ptr<textline>> & lines);

		// returns the input vector without the string parameter, used for compatibility purposes
		std::vector<std::unique_ptr<BOX>> remove_string_from_pair(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & input);

		// returns true if the given word has no meaningful context
		bool is_word_empty(const char* word);

		// returns the iterator to the place in a sorted vector where the value is higher that the given constant
		std::vector<int>::iterator page::get_val_above_constant(std::vector<int> & all_spaces, double constant);

		// returns true when given lines are too far away from each other to be in the same table
		bool rows_in_different_tables(std::shared_ptr<textline> & first, std::shared_ptr<textline> & second);

		// returns true if first[iter_one] is in the same column as second[iter_two] after one iteration
		bool find_same_column(size_t & iter_one, size_t & iter_two, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> * first, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> * second);

		// returns true if given boxes can be merged and do not overlap with anything else
		bool can_merge_columns(size_t & iter_one, size_t & iter_two, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> * first, std::vector<std::pair<std::unique_ptr<BOX>, std::string>> * second);

		// returns a whitespace between rows given lines in the same table
		int get_row_whitespace(std::vector<std::shared_ptr<textline>> lines);
	};

}