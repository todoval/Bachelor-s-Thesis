#include "utils.h"


int centre(std::unique_ptr<BOX> & box)
{
	return box->x + box->w / 2;
}

int get_y_axis(std::vector<std::unique_ptr<BOX>> & input)
{
	auto min_y = std::min_element(input.begin(), input.end(), [](std::unique_ptr<BOX> & a, std::unique_ptr<BOX> & b) {return a->y < b->y;  });
	return (*min_y)->y;
}

std::string get_filename(const std::string & input_path)
{
	int start = input_path.find_last_of("/\\");
	int end = input_path.find_last_of(".");
	return input_path.substr(start + 1, end - start - 1);
}

Pix *matToPix(cv::Mat *mat)
{
	Pix *pixd = pixCreate(mat->size().width, mat->size().height, 32);
	for (int y = 0; y < mat->rows; y++) {
		for (int x = 0; x < mat->cols; x++) {
			pixSetPixel(pixd, x, y, (l_uint32)mat->at<uchar>(y, x));
		}
	}
	return pixd;
}

int most_common_number(std::vector<int> & numbers)
{
	if (numbers.empty())
		return 0;
	std::pair <int, int> result = std::pair<int, int>(0,0);
	std::pair <int, int> current = std::pair<int, int>(numbers[0], 1);

	for (size_t i = 1; i < numbers.size(); i++)
	{
		if (numbers[i] == current.first)
			current.second++;
		else
		{
			if (result.second < current.second)
			{
				result.second = current.second;
				result.first = current.first;
			}
			current.first = numbers[i];
			current.second = 1;
		}
	}
	if (result.second < current.second)
		result.first = current.first;
	return result.first;
}

double get_multi_factor(int space_width, double constant)
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

bool overlap(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second)
{
	return ((second->x < first->x + first->w && second->x > first->x)
		|| (first->x < second->x + second->w && first->x > second->x));
}

int get_char_height(line & symbols, int img_width)
{
	line filtered;
	for (const auto & elem : symbols)
	{
		if (elem->w > 5 && elem->w < img_width / 2)
			filtered.push_back(std::unique_ptr<BOX>(new BOX(*elem.get())));
	}
	if (filtered.empty())
		return 0;
	auto highest_box = std::max_element(filtered.begin(), filtered.end(), [&img_width](std::unique_ptr<BOX> & a, std::unique_ptr<BOX> & b)
		{return a->h < b->h; });
	return (*highest_box)->h;
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
		|| abs(centre(first) - centre(second)) <= COL_THRESHOLD * 5)
		/*&& diff_h*/);
}

bool is_most_left(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second)
{
	return first->x < second->x;
}
