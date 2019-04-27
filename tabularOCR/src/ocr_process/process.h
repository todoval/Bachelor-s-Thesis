#include "utils.h"

namespace tabular_ocr
{
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
			bbox box;
			std::vector<boxed_string> symbols;
			std::vector<boxed_string> columns;

			/*
			textline(const textline &) = delete;
			textline & operator=(const textline&) = delete;
			textline& operator=(textline&& other)
			{
				font = std::move(other.font);
				word_ws = std::move(other.word_ws);
				col_ws = std::move(other.col_ws);
				symbols = std::move(other.symbols);
				columns = std::move(other.columns);
				box = other.box;
				return *this;
			}
			*/
			textline();

			bool operator==(const textline & other) const
			{
				return (other.col_ws == col_ws && other.word_ws == word_ws && font == other.font && box == other.box
					&& symbols.size() == other.symbols.size() && columns.size() == other.columns.size());
			}
		};

		class cell
		{
		public:
			std::string text;
			bbox box;
			int rows_no;
			int cols_no;

			cell();

			cell(const cell &) = delete;
			cell & operator=(const cell&) = delete;
			cell& operator=(cell&& other)
			{
				text = std::move(other.text);
				box = std::move(other.box);
				rows_no = std::move(other.rows_no);
				cols_no = std::move(other.cols_no);
				return *this;
			}
			cell(cell && other)
			{
				text = std::move(other.text);
				box = std::move(other.box);
				rows_no = std::move(other.rows_no);
				cols_no = std::move(other.cols_no);
			}

		};

		class table
		{
		public:
			size_t rows;
			size_t cols;

			std::vector<bbox> row_repres;
			std::vector<bbox> column_repres;
			bbox table_repres;
			std::vector<cell> cells;

			std::vector<textline> textlines;

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
			image img_old;
			image img_preprocessed;
			std::vector<table> all_tables;
			std::vector<textline> textlines;

			page(const page &) = delete;
			page & operator=(const page&) = delete;
			page();
			page(file_info & file);
			~page()
			{

			};

			// colors the border of the given box in the current image with the given rgb color
			void set_border(const bbox & box, int r, int g, int b);
			// the main function
			void process_image();

		private:
			std::unique_ptr<tesseract::TessBaseAPI> api;
			std::string filename;

			// assigns each textline it's columns and whitespaces
			void determine_columns();

			// initializes tesseract api without the use of LSTM
			void init_api(image &img);

			// gets the symbols and lines recognized by tesseract and initializes class's textlines vector with recognized values
			void init_textlines();

			// returns true if a symbol (defined by a box) is in textline (defined by another box)
			bool is_symbol_in_textline(bbox & symbol, bbox & textline);

			// if a footer on the page exists, it removes it
			void delete_footer();

			// merges two given boxes horizontally and returns the result in the result parameter
			void box_merge_horizontal(boxed_string & result, const boxed_string & to_add, bool add_space);

			// merges two given boxes vertically and returns the result in the result parameter
			void box_merge_vertical(boxed_string & result, const boxed_string & to_add);

			//merges columns into tables and saves them into the all_tables vector 
			void create_tables_from_cols();

			// returns a merged vector of words given a vector of symbols and whitespace between words in a single textline
			std::vector<boxed_string> merge_into_words(const std::vector<boxed_string> & symbols, int whitespace);

			// returns a merged vector of columns given a vector of words and whitespace between columns in a single textline
			std::vector<boxed_string> merge_into_columns(const std::vector<boxed_string> & words, int whitespace);

			// returns a vector of whitespaces that exist between given symbols - should not be used on multiline textlines
			std::vector<int> get_spaces(const std::vector<boxed_string> & symbols);

			// returns a pair representing whitespace between words and whitespace between columns given the spaces in a textline
			std::pair<int, int> get_whitespaces(std::vector<int> & all_spaces, double constant);

			// merges two given lines according to given map that tells which pairs of boxes should be merged
			std::vector<boxed_string> merge_lines(const std::vector<boxed_string> & first, const std::vector<boxed_string> & second, std::map<int, int> & no_of_cols);

			// removes textlines that are unimportant for recognition from the textline vector
			void delete_unusual_lines();

			// returns a box representation of a table
			bbox merge_to_table_box(const std::vector<boxed_string> & cols);

			// initialize the given table with the information given by the second parameter
			void init_table(table & curr_table, std::vector<boxed_string> & merged_cols);

			// returns true if the line recognized by tesseract is too big to be a simple textline - probably a badly recognized table
			bool is_textline_table(const textline & line);

			// returns true if given textlines from recognized table are close enough to be in a same cell together
			bool are_in_same_row(const textline & first, const textline & second, int whitespace);

			// returns the vector of cells in a table defined by rows and columns
			std::vector<cell> create_cells(std::vector<textline> & row, std::vector<boxed_string> & merged_cols);

			// appends the source vector to the dest vector
			void move_append(std::vector<cell>& dest, std::vector<cell>& source);

			// returns the height of the input textlines merged together
			int get_merged_lines_height(const std::vector<textline> & lines);

			// returns the input vector without the string parameter, used for compatibility purposes
			std::vector<bbox> remove_string_from_pair(const std::vector<boxed_string> & input);

			// returns true if the given word has no meaningful context
			bool is_word_empty(const std::string& word);

			// returns the iterator to the place in a sorted vector where the value is higher that the given constant
			std::vector<int>::iterator get_val_above_constant(std::vector<int> & all_spaces, double constant);

			// returns true when given lines are too far away from each other to be in the same table
			bool rows_in_different_tables(const textline & first, const textline & second);

			// returns true if first[iter_one] is in the same column as second[iter_two] after one iteration
			bool find_same_column(size_t & iter_one, size_t & iter_two, std::vector<boxed_string> * first, std::vector<boxed_string> * second);

			// returns true if given boxes can be merged and do not overlap with anything else
			bool can_merge_columns(size_t & iter_one, size_t & iter_two, std::vector<boxed_string> * first, std::vector<boxed_string> * second);

			// returns a whitespace between rows given lines in the same table
			int get_row_whitespace(const std::vector<textline> lines);

			// sets the borders of all cells in all tables of the image
			void set_table_borders();

			// sets borders to all the cells in the given table
			void set_cell_borders(const table & table);
		};

	}
}
