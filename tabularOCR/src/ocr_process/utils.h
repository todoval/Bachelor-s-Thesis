#include "../preprocessing/preprocess.h"

#ifdef _WIN32

#include <baseapi.h>
#include <renderer.h>

#else

#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>

#endif // _WIN32

namespace tabular_ocr
{
	class bbox
	{
	public:
		l_int32 x;
		l_int32 y;
		l_int32 h;
		l_int32 w;

		bbox();
		bbox(Box* box);

		bool operator==(const bbox & other) const
		{
			return (x == other.x && y == other.y && w == other.w && h == other.h);
		}

		bool not_initialized();
	};

	struct boxed_string
	{
		bbox box;
		std::string text;
	};

	typedef std::vector<bbox> line;
	typedef std::vector<bbox> column;

	const size_t COL_THRESHOLD = 10;

	// returns the centre on x coordinate of a given box
	int centre(const bbox & box);

	// returns the minimal y value that exists in the given line
	int get_y_axis(const std::vector<boxed_string> & input);

	// given symbol spacing of a line and a constant for normalization of space, determine the multiplication factor between words using a pseudo logarithmic curve
	double get_multi_factor_words(int space_width, double constant);

	// given word spacing of a line, determine the multiplication factor between columns using a pseudo logarithmic curve
	double get_multi_factor_columns(int space_width);

	// returns true if the given boxes overlap each other
	bool overlap(const bbox & first, const bbox & second);

	// returns the greatest height of line
	int get_char_height(const std::vector<boxed_string> & symbols, int img_width);

	// returns the width of a column defined by two boxes
	int get_width_of_col(const bbox & first, const bbox & second);

	// returns true if given boxes are in same column
	bool are_in_same_col(const bbox & first, const bbox & second);

	// returns true if first box is more left than the second
	bool is_most_left (const bbox & first, const bbox & second);

	// sorts the input vector by the x coordinate of the box in one element
	void sort_by_xcoord(std::vector<boxed_string> & input);

}