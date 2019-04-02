#include <vector>
#include <algorithm> 
#include "utils.h"
#include <unordered_map>

namespace ocr
{

	const int FOOTER_THRESHOLD = 15;

	const double REF_FONT_SIZE = 19;

	class textline
	{
	public:
		size_t font;
		size_t whitespace;
		std::vector<BOX*> symbols;
		std::vector<BOX*> columns;

		textline();

		bool operator==(const textline & other) const
		{
			return (other.whitespace == whitespace
				&& symbols.size() == other.symbols.size() && columns.size() == other.columns.size());
		}
	};

	class table
	{
	public:
		size_t rows;
		size_t cols;
		
		std::vector<BOX*> row_repres;
		std::vector<BOX*> column_repres;
		BOX * table_repres;
		
		std::vector<textline*> textlines;

		table();
		
	};

	class page
	{
	public:
		Pix *img;
		std::vector<table> all_tables;
		std::vector<textline> textlines;

		page();
		page(const std::string & filename);
		void set_border(BOX *box, int r, int g, int b);
		void process_image();

	private:
		tesseract::TessBaseAPI *api;

		void determine_columns(std::multimap<int, textline*> & fonts);

		void init_api(Pix *img);
		std::multimap<int, textline*> init_textlines();

		// returns true if a symbol (defined by a box) is in textline (defined by another box)
		bool is_symbol_in_textline(BOX* symbol, BOX* textline);
		void delete_footer();
		void box_merge_horizontal(BOX* result, BOX* to_add);
		void box_merge_vertical(BOX* result, BOX* to_add);
		std::vector<table> merge_cols(std::vector<textline> & page);
		std::vector<BOX*> merge_into_words(std::vector<BOX*> & symbols, int whitespace);
		/*
		returns a vector of whitespaces that exist between the symbols given
		the vector of symbols should represent one line or a part of line
		*/
		std::vector<int> get_spaces(const line & symbols);

		std::vector<BOX*> page::merge_into_columns(std::vector<BOX*> & words, int whitespace);

		// returns the whitespace between words in textline
		int get_whitespace(std::vector<int> & all_spaces, double constant);

		line merge_lines(line & first, line & second, std::map<int,int> & no_of_cols);

		void process_category(int & cat_font, std::vector<textline *> & cat_lines, int & first_val);

		void page::process_cat_and_init(int & cat_font, std::vector<textline *> & cat_lines, int & first_val,
			std::multimap<int, textline * >::iterator it);

		void delete_unusual_lines();

		BOX* merge_to_table(std::vector<BOX*> & cols);

		void create_table(table & curr_table, std::vector<BOX*> & merged_cols);

		bool is_textline_table(textline & line);
	};

}