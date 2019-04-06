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

int centre(std::unique_ptr<BOX> & box);

int get_y_axis(std::vector<std::unique_ptr<BOX>> & input);

std::string get_filename(const std::string & input_path);

Pix *matToPix(cv::Mat *mat);

int most_common_number(std::vector<int> & numbers);

double get_multi_factor_words(int space_width, double constant);

bool overlap(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second);

int get_char_height(line & symbols, int img_width);

int get_width_of_col(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second);

bool are_in_same_col(std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second);

bool is_most_left (std::unique_ptr<BOX> & first, std::unique_ptr<BOX> & second);