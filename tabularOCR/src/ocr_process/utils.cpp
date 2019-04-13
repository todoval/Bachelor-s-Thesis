#include "utils.h"


int centre(std::unique_ptr<BOX> & box)
{
	return box->x + box->w / 2;
}

int get_y_axis(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & input)
{
	auto min_y = std::min_element(input.begin(), input.end(), [](auto & a, auto & b) {return a.first->y < b.first->y;  });
	return (*min_y).first->y;
}

double get_multi_factor_words(int space_width, double constant)
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

double get_multi_factor_columns(int space_width)
{
	if (space_width > 5)
		return 3.5;
	else if (space_width <= 5 && space_width >= 3)
		return ((5 - space_width) / 2 + 4);
	else if (space_width < 3 && space_width >= 2)
		return ( 7 - space_width );
	else if (space_width < 2)
		return 10;
	return 0.0;
}

bool overlap(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second)
{
	return ((second->x <= first->x + first->w && second->x >= first->x)
		|| (first->x <= second->x + second->w && first->x >= second->x));
}

int get_char_height(std::vector<std::pair<std::unique_ptr<BOX>, std::string>>  & symbols, int img_width)
{
	if (symbols.empty())
		return 0;
	auto highest_box = std::max_element(symbols.begin(), symbols.end(), [](std::pair<std::unique_ptr<BOX>, std::string> & a, std::pair<std::unique_ptr<BOX>, std::string> & b)
		{return a.first->h < b.first->h; });
	return (*highest_box).first->h;
}

int get_width_of_col(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second)
{
	int first_part = abs(first->x - second->x);
	int sec_part = std::max(first->x + first->w, second->x + second->w) - std::max(first->x, second->x);
	return first_part + sec_part;
}

bool are_in_same_col(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second)
{
	bool diff_h;
	if (first->y < second->y)
		diff_h = second->y - first->y - first->h < std::min(first->h, second->h);
	else
		diff_h = first->y - second->y - second->h < std::min(first->h, second->h);
	return ((abs(first->x - second->x) <= COL_THRESHOLD
		|| abs(first->x + first->w - (second->x + second->w)) <= COL_THRESHOLD
		|| abs(centre(first) - centre(second)) <= COL_THRESHOLD * 5) );
}

bool is_most_left(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second)
{
	return first->x < second->x;
}

void sort_by_xcoord(std::vector<std::pair<std::unique_ptr<BOX>, std::string>>& input)
{
	std::sort(input.begin(), input.end(),
		[](auto & a, auto & b) { return a.first->x < b.first->x; });
}