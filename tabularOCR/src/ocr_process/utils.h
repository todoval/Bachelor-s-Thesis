#include "opencv2/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "../preprocessing/preprocess.h"

#ifdef _WIN32

#include <baseapi.h>
#include <renderer.h>

#else

#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>

#endif // _WIN32

typedef std::vector<std::unique_ptr<BOX>> line;
typedef std::vector<std::unique_ptr<BOX>> column;

const size_t COL_THRESHOLD = 10;

// returns the centre on x coordinate of a given box
int centre(std::unique_ptr<BOX> & box);

// returns the minimal y value that exists in the given line
int get_y_axis(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & input);

// returns the filename from a given full input path
std::string get_filename(const std::string & input_path);

// given symbol spacing of a line and a constant for normalization of space, determine the multiplication factor between words using a pseudo logarithmic curve
double get_multi_factor_words(int space_width, double constant);

// given word spacing of a line, determine the multiplication factor between columns using a pseudo logarithmic curve
double get_multi_factor_columns(int space_width);

// returns true if the given boxes overlap each other
bool overlap(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second);

// returns the greatest height of line
int get_char_height(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & symbols, int img_width);

// returns the width of a column defined by two boxes
int get_width_of_col(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second);

// returns true if given boxes are in same column
bool are_in_same_col(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second);

// returns true if first box is more left than the second
bool is_most_left (std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second);

// sorts the input vector by the x coordinate of the box in one element
void sort_by_xcoord(std::vector<std::pair<std::unique_ptr<BOX>, std::string>> & input);