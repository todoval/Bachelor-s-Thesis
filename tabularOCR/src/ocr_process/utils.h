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

typedef std::vector<BOX*> line;
typedef std::vector<BOX*> column;

const size_t COL_THRESHOLD = 10;

int centre(BOX* box);

int get_y_axis(std::vector<BOX*> & input);

std::string get_filename(const std::string & input_path);

Pix *matToPix(cv::Mat *mat);

int most_common_number(std::vector<int> & numbers);

double get_multi_factor(int space_width, double constant);

bool overlap(BOX* first, BOX* second);

int get_char_height(line & symbols, int img_width);

int get_width_of_col(BOX* first, BOX* second);

bool are_in_same_col(BOX* first, BOX* second);

bool is_most_left (BOX * first, BOX * second);