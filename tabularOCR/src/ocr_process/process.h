#include <vector>
#include <algorithm> 
#include "utils.h"
#include <memory>
#include <unordered_map>

namespace ocr
{

	const int FOOTER_THRESHOLD = 15;

	const double REF_FONT_SIZE = 19;

	class textline
	{
	public:
		size_t font;
		size_t word_ws;
		size_t col_ws;
		std::vector<std::unique_ptr<BOX>> symbols;
		std::vector<std::unique_ptr<BOX>> columns;

		textline(const textline &) = delete;
		textline & operator=(const textline&) = delete;
		textline& operator=(textline&& other)
		{
			font = std::move(other.font);
			word_ws = std::move(other.word_ws);
			col_ws = std::move(other.col_ws);
			symbols = std::move(other.symbols);
			columns = std::move(other.columns);
			return *this;
		}

		textline();

		bool operator==(const textline & other) const
		{
			return (other.col_ws == col_ws && other.word_ws == word_ws
				&& symbols.size() == other.symbols.size() && columns.size() == other.columns.size());
		}
	};

	class table
	{
	public:
		size_t rows;
		size_t cols;
		
		table(const table &) = delete;
		table(table && other)
		{
			rows = std::move(other.rows);
			cols = std::move(other.cols);
			row_repres = std::move(other.row_repres);
			column_repres = std::move(other.column_repres);
			table_repres = std::move(other.table_repres);
			textlines = std::move(other.textlines);
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

		std::vector<std::unique_ptr<BOX>> row_repres;
		std::vector<std::unique_ptr<BOX>> column_repres;
		std::unique_ptr<BOX> table_repres;
		
		std::vector<std::shared_ptr<textline>> textlines;

		table();
		
	};

	class page
	{
	public:
		Pix *img;
		std::vector<table> all_tables;
		std::vector<std::shared_ptr<textline>> textlines;

		page(const page &) = delete;
		page & operator=(const page&) = delete;

		page();
		page(const std::string & filename);
		~page()
		{
			
		};
		void set_border(std::unique_ptr<BOX> & box, int r, int g, int b);
		void process_image();

	private:
		tesseract::TessBaseAPI *api;
		std::string filename;

		void determine_columns(std::multimap<int, std::shared_ptr<textline>> & fonts);

		void init_api(Pix *img);
		std::multimap<int, std::shared_ptr<textline>> init_textlines();

		// returns true if a symbol (defined by a box) is in textline (defined by another box)
		bool is_symbol_in_textline(std::unique_ptr<BOX> & symbol, std::unique_ptr<BOX> & textline);
		void delete_footer();
		void box_merge_horizontal(std::unique_ptr<BOX> & result, std::unique_ptr<BOX> & to_add);
		void box_merge_vertical(std::unique_ptr<BOX> & result, std::unique_ptr<BOX> & to_add);
		std::vector<table> merge_cols(std::vector<std::shared_ptr <textline>> & page);
		std::vector<std::unique_ptr<BOX>> merge_into_words(std::vector<std::unique_ptr<BOX>> & symbols, int whitespace);
		/*
		returns a vector of whitespaces that exist between the symbols given
		the vector of symbols should represent one line or a part of line
		*/
		std::vector<int> get_spaces(const std::vector<std::unique_ptr<BOX>> & symbols);

		std::vector<std::unique_ptr<BOX>> page::merge_into_columns(std::vector<std::unique_ptr<BOX>> & words, int whitespace);

		// returns the whitespace between words in textline
		std::pair<int, int> get_whitespace(std::vector<int> & all_spaces, double constant);

		line merge_lines(line & first, line & second, std::map<int,int> & no_of_cols);

		void process_category(int & cat_font, std::vector<std::shared_ptr<textline>> & cat_lines, int & first_val);

		void process_cat_and_init(int & cat_font, std::vector<std::shared_ptr<textline>> & cat_lines, int & first_val,
			std::multimap<int, std::shared_ptr<textline> >::iterator it);

		void delete_unusual_lines();

		int get_column_whitespace(std::vector<int> & word_gaps);

		std::unique_ptr<BOX> merge_to_table(std::vector<std::unique_ptr<BOX>> & cols);

		void create_table(table & curr_table, std::vector<std::unique_ptr<BOX>> & merged_cols);

		bool is_textline_table(std::shared_ptr<textline> line);
	};

}