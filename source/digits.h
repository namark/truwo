#ifndef DIGITS_H
#define DIGITS_H

#include "bitmap.hpp"

using digit_data = bitmap_data<3,5>;
extern const std::array<digit_data, 10> digit;

#endif /* end of include guard */
