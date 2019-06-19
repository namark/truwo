#ifndef DIGITS_H
#define DIGITS_H

#include "bitmap.hpp"

using digit_data = bitmap_data<3,5>;
using digit_bitmap = bitmap<3,5>;

const size_t digit_count = 11;

extern const std::array<digit_data, digit_count> digit;

#endif /* end of include guard */
