#include "utils.h"

bool tabular_ocr::bbox::not_initialized() const
{
	return (x == 0 && y == 0 && h == 0 && w == 0);
}

int tabular_ocr::centre(const bbox & box)
{
	return box.x + box.w / 2;
}

int tabular_ocr::get_y_axis(const std::vector<boxed_string> & input)
{
	auto min_y = std::min_element(input.begin(), input.end(), [](auto & a, auto & b) {return a.box.y < b.box.y;  });
	return (*min_y).box.y;
}

double tabular_ocr::get_multi_factor_words(int space_width, double constant)
{
	double x = space_width / constant;
	if (x >= 4)
		return 1.5;
	if (x < 4 && x >= 3)
		return ((4 - x)* 0.1 + 1.5);
	if (x < 3 && x >= 2)
		return ((3 - x) * 0.4 + 1.6);
	if (x < 2 && x >= 1)
		return (4 - x);
	return 0;
}

double tabular_ocr::get_multi_factor_columns(int space_width)
{
	if (space_width > 5)
		return 3.5;
	else if (space_width <= 5 && space_width >= 3)
		return ((5 - space_width) / 2 + 4);
	else if (space_width < 3 && space_width >= 2)
		return (7 - space_width);
	else if (space_width < 2)
		return 10;
	return 0.0;
}

bool tabular_ocr::overlap(const bbox & first, const bbox & second)
{
	return ((second.x <= first.x + first.w && second.x >= first.x)
		|| (first.x <= second.x + second.w && first.x >= second.x));
}

int tabular_ocr::get_char_height(const std::vector<boxed_string> & symbols, int img_width)
{
	if (symbols.empty())
		return 0;
	auto highest_box = std::max_element(symbols.begin(), symbols.end(), [](const boxed_string & a, const boxed_string & b)
	{return a.box.h < b.box.h; });
	return (*highest_box).box.h;
}

int tabular_ocr::get_width_of_col(const bbox & first, const bbox & second)
{
	int first_part = abs(first.x - second.x);
	int sec_part = std::max(first.x + first.w, second.x + second.w) - std::max(first.x, second.x);
	return first_part + sec_part;
}

bool tabular_ocr::are_in_same_col(const bbox & first, const bbox & second)
{
	bool diff_h;
	if (first.y < second.y)
		diff_h = second.y - first.y - first.h < std::min(first.h, second.h);
	else
		diff_h = first.y - second.y - second.h < std::min(first.h, second.h);
	return ((abs(first.x - second.x) <= COL_THRESHOLD
		|| abs(first.x + first.w - (second.x + second.w)) <= COL_THRESHOLD
		|| abs(centre(first) - centre(second)) <= COL_THRESHOLD * 5));
}

bool tabular_ocr::is_most_left(const bbox & first, const bbox & second)
{
	return first.x < second.x;
}

void tabular_ocr::sort_by_xcoord(std::vector<boxed_string>& input)
{
	std::sort(input.begin(), input.end(),
		[](auto & a, auto & b) { return a.box.x < b.box.x; });
}
